#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iterator>
#include <limits>
#include <numeric>
#include <iostream>
#include <fstream>
#include <string>

#include "cache.h"
#include "util.h"

enum CLASSES { NONREUSED = 0, REUSED = 1, NUM_CLASSES };

struct Class {
  // ranks, higher is better; could be stored as eviction order instead
  std::vector<double> ranks;

  // monitor (hardware counters)
  std::vector<uint64_t> hits;
  std::vector<uint64_t> evictions;

  // averaged stats for computing ranks (software counters)
  std::vector<double> ewmaHits;
  std::vector<double> ewmaEvictions;

  // debugging
  uint64_t cumulativeHits;
  uint64_t cumulativeEvictions;
  // the following three std::vectors are for debug output *only* and
  // could easily be optimized out in reconfigure() -nzb
  std::vector<double> hitProbability;
  std::vector<double> expectedLifetime;
  std::vector<double> opportunityCost;

  /* Class(FeedbackBaseReplPolicy*);
                // virtual ~Class();

                // static Class* create(FeedbackBaseReplPolicy* _owner, std::string _type);

                // void update();
                // virtual void reconfigure(double lineGain) = 0;
                // void reset();
                // void dumpStats();
                */
};

std::vector<uint64_t> timestamps;
std::vector<uint32_t> classIds;
std::vector<Class*> classes;

uint64_t now;
uint64_t accsPerInterval = 2 * 1024; // Need to confirm
uint64_t maxAge = 500;               // Need to find value
double ewmaDecay = 1;                // Need to find value
uint64_t ageScaling = 1;               // Need to find value
uint64_t nextUpdate = accsPerInterval;
uint64_t numLines;
uint64_t wrapArounds;
std::ofstream output_file(std::to_string(maxAge)+"eva_ages.txt", std::ios::out | std::ios::trunc);

// Remember to turn off prefetching in the config file
void CACHE::initialize_replacement()
{ 

  classes.resize(NUM_CLASSES);
  classes[NONREUSED] = new Class;
  classes[REUSED] = new Class;
  numLines = NUM_SET * NUM_WAY;
  classIds.resize(numLines, 0);
  timestamps.resize(numLines, 0);
  now = 0;
  wrapArounds = 0;
  std::fill(classIds.begin(), classIds.end(), NONREUSED); // Is it required?
  for (auto cl : classes) {
    cl->ranks.resize(maxAge, 0.);

    cl->hits.resize(maxAge, 0);
    cl->evictions.resize(maxAge, 0);

    cl->ewmaHits.resize(maxAge, 0.);
    cl->ewmaEvictions.resize(maxAge, 0.);

    cl->cumulativeHits = 0;
    cl->cumulativeEvictions = 0;
    cl->hitProbability.resize(maxAge, 0.);
    cl->expectedLifetime.resize(maxAge, 0.);
    cl->opportunityCost.resize(maxAge, 0.);
  }
}
uint64_t age(uint32_t cache_lineID)
{ // Zsim Aging->age()
  uint64_t exact = now - timestamps[cache_lineID];
  uint64_t coarse = exact / ageScaling;
  uint64_t mod = coarse % maxAge;
  return mod;
}

void UpdtAge(uint32_t cache_lineID)
{ // Zsim Aging->update()
  now++;

  // check for wraparounds
  uint64_t exact = now - timestamps[cache_lineID]; // VTS - exact is the lifetime
  uint64_t coarse = exact / ageScaling;
  if (coarse > maxAge) {
    ++(wrapArounds);
  }

  timestamps[cache_lineID] = now;
}

void reset()
{
  for (auto* cl : classes) {
    std::fill(cl->hits.begin(), cl->hits.end(), 0);
    std::fill(cl->evictions.begin(), cl->evictions.end(), 0);
  }
  nextUpdate += accsPerInterval;
  wrapArounds = 0;
}

void update_cl(Class* cl)
{
  for (uint32_t a = 0; a < maxAge; a++) {
    cl->ewmaHits[a] *= ewmaDecay;
    cl->ewmaHits[a] += cl->hits[a];

    cl->ewmaEvictions[a] *= ewmaDecay;
    cl->ewmaEvictions[a] += cl->evictions[a];
  }
}

double ageScaler(uint64_t a)
{
  if (a == 0) {
    return ageScaling / 2. + 0.5;
  } else {
    return ageScaling;
  }
}

void eva_calc(double lineGain, Class* cl)
{

  std::vector<double> events(maxAge);
  std::vector<double> totalEventsAbove(maxAge + 1);
  totalEventsAbove[maxAge] = 0.;

  for (uint32_t a = maxAge - 1; a < maxAge; a--) { // VTS : a is uint and will wrap around after it crosses zero, effectively terminating the loop
    events[a] = cl->ewmaHits[a] + cl->ewmaEvictions[a];
    totalEventsAbove[a] = totalEventsAbove[a + 1] + events[a];
  }

  uint32_t a = maxAge - 1;
  cl->hitProbability[a] = (totalEventsAbove[a] > 1e-2) ? 0.5 * cl->ewmaHits[a] / totalEventsAbove[a] : 0.;
  cl->expectedLifetime[a] = ageScaler(a);
  double expectedLifetimeUnconditioned = ageScaler(a) * totalEventsAbove[a];
  double totalHitsAbove = cl->ewmaHits[a];

  // short lines
  //
  // computed assuming events are uniformly distributed within each
  // coarsened region.
  for (uint32_t a = maxAge - 2; a < maxAge; a--) {
    if (totalEventsAbove[a] > 1e-2) {
      cl->hitProbability[a] = (0.5 * cl->ewmaHits[a] + totalHitsAbove) / (0.5 * events[a] + totalEventsAbove[a + 1]);
      cl->expectedLifetime[a] = ((1. / 6) * ageScaler(a) * events[a] + expectedLifetimeUnconditioned) / (0.5 * events[a] + totalEventsAbove[a + 1]);
      // info("ageScaler at %u is a %g", a, aging->ageScaler(a));
    } else {
      cl->hitProbability[a] = 0.;
      cl->expectedLifetime[a] = 0.;
    }

    totalHitsAbove += cl->ewmaHits[a];
    expectedLifetimeUnconditioned += ageScaler(a) * totalEventsAbove[a];
  }

  // finally, compute EVA from the probabilities and lifetimes
  for (uint32_t a = maxAge - 1; a < maxAge; a--) {
    if (std::isnan(lineGain)) { // VTS : line gain is perAccessCost
      cl->opportunityCost[a] = 0.;
    } else {
      cl->opportunityCost[a] = lineGain * cl->expectedLifetime[a];
    }
    cl->ranks[a] = cl->hitProbability[a] - cl->opportunityCost[a];
  }
}

void reconfigure()
{
  uint64_t intervalHits = 0;
  uint64_t intervalEvictions = 0;
  uint64_t ewmaHits = 0;
  uint64_t ewmaEvictions = 0;
  uint64_t cumHits = 0;
  uint64_t cumEvictions = 0;

  for (auto* cl : classes) { // VTS: Line 2 in Pseudo Code
    update_cl(cl);

    intervalHits += std::accumulate(cl->hits.begin(), cl->hits.end(), 0);
    intervalEvictions += std::accumulate(cl->evictions.begin(), cl->evictions.end(), 0);

    ewmaHits += std::accumulate(cl->ewmaHits.begin(), cl->ewmaHits.end(), 0);
    ewmaEvictions += std::accumulate(cl->ewmaEvictions.begin(), cl->ewmaEvictions.end(), 0);

    cumHits += cl->cumulativeHits;
    cumEvictions += cl->cumulativeEvictions;
  }

  double lineGain = 1. * ewmaHits / (ewmaHits + ewmaEvictions) / numLines; // VTS : Algorithm Line 3: (1-m)/S -> Hit_Rate/Cache_Size

  for (auto* cl : classes) {
    eva_calc(lineGain, cl);

    // it can never be allowed for the saturating age to have highest
    // rank, or the cache can get stuck with all lines saturated
    cl->ranks[maxAge - 1] = std::numeric_limits<double>::lowest();
  }

  // uint64_t newAgeScaling = ageScaling;
  // newAgeScaling = aging->adaptAgeScale();

  // with compressed arrays, fills aren't recorded and one access can trigger multiple evictions
  // uint64_t accesses = fills + intervalHits + intervalEvictions;
  // assert(accesses == accsPerInterval);

  // ageScaling = newAgeScaling;
}

void reconfigure_cl()
{
  reconfigure();

  Class* rc = classes[REUSED];
  Class* nc = classes[NONREUSED];

  // VTS : Calculations for lines 11 & 12
  uint64_t reusedHits = std::accumulate(rc->ewmaHits.begin(), rc->ewmaHits.end(), 1);
  uint64_t reusedEvictions = std::accumulate(rc->ewmaEvictions.begin(), rc->ewmaEvictions.end(), 1);
  double reusedMissRate = 1. * reusedEvictions / (reusedHits + reusedEvictions);

  uint64_t nonReusedHits = std::accumulate(nc->ewmaHits.begin(), nc->ewmaHits.end(), 1);
  uint64_t nonReusedEvictions = std::accumulate(nc->ewmaEvictions.begin(), nc->ewmaEvictions.end(), 1);
  double nonReusedMissRate = 1. * nonReusedEvictions / (nonReusedHits + nonReusedEvictions);

  uint64_t totalHits = reusedHits + nonReusedHits;
  uint64_t totalEvictions = reusedEvictions + nonReusedEvictions;
  double averageMissRate = 1. * totalEvictions / (totalHits + totalEvictions);

  double reusedLifetimeBias = rc->ranks[0];

  for (auto* cl : classes) { // VTS: LInes 11 - 15 in Pseudo Code
    for (uint32_t a = maxAge - 1; a < maxAge; a--) {
      cl->ranks[a] += (averageMissRate - (1 - cl->hitProbability[a])) / reusedMissRate * reusedLifetimeBias;
    }
  }
}

void update(uint32_t cache_lineID, uint8_t hit)
{
  if (hit) {

    uint32_t classID = classIds[cache_lineID];
    classes[classID]->hits[age(cache_lineID)]++;
  }

  // age update
  UpdtAge(cache_lineID);

  if (--nextUpdate == 0) {
    reconfigure_cl();
      for (auto* cl : classes) { // VTS: LInes 11 - 15 in Pseudo Code
        for (uint32_t a = 0; a < maxAge; a++) {
          output_file << a <<","<< cl->ranks[a] << "\n";
        }
      }
    reset();
  }
}

// find replacement victim - Called in two places - handle_fill() and handle_writeback().
uint32_t CACHE::find_victim(uint32_t cpu, uint64_t instr_id, uint32_t set, const BLOCK* current_set, uint64_t ip, uint64_t full_addr,
                            uint32_t type) // Equivalent to rank() in zsim
{
  // baseline
  uint32_t bestCand = -1;
  double bestRank = std::numeric_limits<double>::max(); // Least rank is the replacement candidate
  uint32_t begin = set * NUM_WAY;
  uint32_t end = (set + 1) * NUM_WAY;
  uint32_t cache_lineID;
  double rank;
  uint64_t a;
  uint32_t classID;
  for (cache_lineID = begin; cache_lineID < end; cache_lineID++) {
    if (!block[cache_lineID].valid) {
      bestCand = cache_lineID;
      break;
    } else {
      a = age(cache_lineID);
      classID = classIds[cache_lineID];
      rank = classes[classID]->ranks[a];
      if (rank < bestRank - 1e-3) {
        bestCand = cache_lineID;
        bestRank = rank;
      }
    }
  }
  // Replaced()

  // replaced2()
  if (timestamps[cache_lineID] != 0) { // Not invalid
    uint32_t classID = classIds[cache_lineID];
    classes[classID]->evictions[age(cache_lineID)]++;
  }
  timestamps[cache_lineID] = 0;
  classIds[cache_lineID] = NONREUSED;
  // cout << "Best Cand :" <<  bestCand - begin << "\n";
  return bestCand - begin;
}

// called on every cache hit and cache fill
void CACHE::update_replacement_state(uint32_t cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type,
                                     uint8_t hit) // Equivalent to both update() and replaced() in zsim
{

  if (hit && type == WRITEBACK) // need to understand
    return;
  uint32_t cache_lineID = (set * NUM_WAY) + way;
  update(cache_lineID, hit); // Equivalent to update2 in ZSIM
  classIds[cache_lineID] = hit ? REUSED : NONREUSED;
}

void CACHE::replacement_final_stats() {}
