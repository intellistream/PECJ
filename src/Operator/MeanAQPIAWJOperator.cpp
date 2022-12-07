
#include <Operator/MeanAQPIAWJOperator.h>
#include <JoinAlgos/JoinAlgoTable.h>
bool OoOJoin::MeanAQPIAWJOperator::setConfig(INTELLI::ConfigMapPtr cfg) {
  if (!OoOJoin::AbstractOperator::setConfig(cfg)) {
    return false;
  }
  return true;
}
bool OoOJoin::MeanAQPIAWJOperator::start() {
  /**
  * @brief set watermark generator
  */
  wmGen = newPeriodicalWM();
  wmGen->setConfig(config);
  wmGen->syncTimeStruct(timeBaseStruct);
  /**
   * @note:
  */
  wmGen->creatWindow(0, windowLen);
  // wmGen->creatWindow(0, windowLen);
  /**
   * @brief set window
   */
  stateOfKeyTableR = newStateOfKeyHashTable(4096, 4);
  stateOfKeyTableS = newStateOfKeyHashTable(4096, 4);
  myWindow.setRange(0, windowLen);
  windowBound = windowLen;
  myWindow.init(sLen, rLen, 1);

  intermediateResult = 0;
  confirmedResult = 0;
  lockedByWaterMark = false;
  return true;
}
void OoOJoin::MeanAQPIAWJOperator::conductComputation() {

}
void OoOJoin::MeanAQPIAWJOperator::updateStateOfKey(MeanStateOfKeyPtr sk, TrackTuplePtr tp) {
  double skewTp = tp->arrivalTime - tp->eventTime;
  double rateTp = tp->arrivalTime;
  rateTp = rateTp / (sk->arrivedTupleCnt + 1);
  //size_t prevUnarrived=0,currentUnarrived=0;
  sk->arrivalSkew = (1 - alphaArrivalSkew) * sk->arrivalSkew + alphaArrivalSkew * skewTp;
  sk->sigmaArrivalSkew = (1 - betaArrivalSkew) * sk->sigmaArrivalSkew + betaArrivalSkew * abs(sk->arrivalSkew - skewTp);
  sk->arrivedTupleCnt++;
  if (sk->lastArrivalTuple == nullptr) {
    sk->lastArrivalTuple = tp;
  } else {
    if (sk->lastArrivalTuple->arrivalTime < tp->arrivalTime) {
      sk->lastArrivalTuple = tp;
    }
  }
  if (sk->lastEventTuple == nullptr) {
    sk->lastEventTuple = tp;
  } else {
    if (sk->lastEventTuple->eventTime < tp->eventTime) {
      sk->lastEventTuple = tp;
    }
  }
}
bool OoOJoin::MeanAQPIAWJOperator::stop() {
  /**
   */
  if (lockedByWaterMark) {
        WM_INFO("early terminate by watermark, already have results");
  }
  if (!lockedByWaterMark) {
        WM_INFO("No watermark encountered, compute now");
  }
  lazyComputeOfAQP();
  size_t rLen = myWindow.windowR.size();
  NPJTuplePtr *tr = myWindow.windowR.data();
  tsType timeNow = lastTimeOfR;
  for (size_t i = 0; i < rLen; i++) {
    if (tr[i]->arrivalTime < timeNow) { tr[i]->processedTime = timeNow; }
  }
  return true;
}
bool OoOJoin::MeanAQPIAWJOperator::feedTupleS(OoOJoin::TrackTuplePtr ts) {
  bool shouldGenWM, isInWindow;
  if (lockedByWaterMark) {
    return false;
  }
  isInWindow = myWindow.feedTupleS(ts);
  shouldGenWM = wmGen->reportTupleS(ts, 1);
  if (shouldGenWM) {
    lockedByWaterMark = true;
    //  return false;

  }
  // bool shouldGenWM;
  if (isInWindow) {
    MeanStateOfKeyPtr sk;
    AbstractStateOfKeyPtr skrf = stateOfKeyTableS->getByKey(ts->key);
    //lastTimeS=ts->arrivalTime;
    if (skrf == nullptr) // this key does'nt exist
    {
      sk = newMeanStateOfKey();
      sk->key = ts->key;
      stateOfKeyTableS->insert(sk);
    } else {
      sk = ImproveStateOfKeyTo(MeanStateOfKey, skrf);
    }
    updateStateOfKey(sk, ts);
    // lazyComputeOfAQP();
  }
  return true;
}

bool OoOJoin::MeanAQPIAWJOperator::feedTupleR(OoOJoin::TrackTuplePtr tr) {
  bool shouldGenWM, isInWindow;
  if (lockedByWaterMark) {
    return false;
  }
  isInWindow = myWindow.feedTupleR(tr);
  shouldGenWM = wmGen->reportTupleR(tr, 1);
  if (shouldGenWM) {
    lockedByWaterMark = true;
    //return false;

  }
  // bool shouldGenWM;
  if (isInWindow) {
    MeanStateOfKeyPtr sk;
    //lastTimeOfR=tr->arrivalTime;
    AbstractStateOfKeyPtr skrf = stateOfKeyTableR->getByKey(tr->key);
    // lastTimeR=tr->arrivalTime;
    if (skrf == nullptr) // this key does'nt exist
    {
      sk = newMeanStateOfKey();
      sk->key = tr->key;
      stateOfKeyTableR->insert(sk);
    } else {
      sk = ImproveStateOfKeyTo(MeanStateOfKey, skrf);
    }
    updateStateOfKey(sk, tr);
    //lazyComputeOfAQP();
  }
  return true;
}
size_t OoOJoin::MeanAQPIAWJOperator::getResult() {
  return confirmedResult;
  // return confirmedResult;
}
double OoOJoin::MeanAQPIAWJOperator::predictUnarrivedTuples(MeanStateOfKeyPtr px) {
  tsType lastTime = px->lastArrivalTuple->arrivalTime;
  double avgSkew = px->arrivalSkew;
  double goThroughTime = lastTime - avgSkew - myWindow.getStart();
  double futureTime = myWindow.getEnd() + avgSkew - lastTime;
  double futureTuple = px->arrivedTupleCnt * futureTime / goThroughTime;
  if (futureTuple < 0) {
    futureTuple = 0;
  }
  return futureTuple;
}
void OoOJoin::MeanAQPIAWJOperator::lazyComputeOfAQP() {
  AbstractStateOfKeyPtr probrPtr = nullptr;
  intermediateResult = 0;
  for (size_t i = 0; i < stateOfKeyTableR->buckets.size(); i++) {
    for (auto iter : stateOfKeyTableR->buckets[i]) {
      if (iter != nullptr) {
        MeanStateOfKeyPtr px = ImproveStateOfKeyTo(MeanStateOfKey, iter);
        probrPtr = stateOfKeyTableS->getByKey(px->key);
        if (probrPtr != nullptr) {
          //  lastTimeS = px->lastArrivalTuple->arrivalTime;
          //lastTimeS=px->lastEventTuple->eventTime;
          double unarrivedS = predictUnarrivedTuples(px);
          MeanStateOfKeyPtr py = ImproveStateOfKeyTo(MeanStateOfKey, probrPtr);
          double unarrivedR = predictUnarrivedTuples(py);
          intermediateResult += (px->arrivedTupleCnt + unarrivedS) * (py->arrivedTupleCnt + unarrivedR);
          /*cout << "S: a=" + to_string(px->arrivedTupleCnt) + ", u=" + to_string(unarrivedS) + "sigma="
              + to_string(px->sigmaArrivalSkew) +
              ";R: a=" + to_string(py->arrivedTupleCnt) + ", u=" + to_string(unarrivedR) + "\n";*/
          confirmedResult += (px->arrivedTupleCnt) * (py->arrivedTupleCnt);

        }
      }
    }
  }
  lastTimeOfR = UtilityFunctions::timeLastUs(timeBaseStruct);
}
size_t OoOJoin::MeanAQPIAWJOperator::getAQPResult() {
  return intermediateResult;
}

