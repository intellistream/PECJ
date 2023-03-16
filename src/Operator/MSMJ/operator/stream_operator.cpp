//
// Created by 86183 on 2023/1/8.
//



#include <future>
#include <utility>
#include "Operator/MSMJ/operator/stream_operator.h"
#include "Operator/MSMJ/common/define.h"
#include "JoinAlgos/JoinAlgoTable.h"
#include "JoinAlgos/AbstractJoinAlgo.h"
#include "WaterMarker/WMTable.h"

using namespace MSMJ;

StreamOperator::StreamOperator(TupleProductivityProfiler *profiler, INTELLI::ConfigMapPtr config) :
        config(std::move(config)), productivity_profiler_(profiler) {
}


bool StreamOperator::start() {
    /**
    * @brief set watermark generator
    */
    //wmGen = newPeriodicalWM();
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
    timeBreakDown_prediction = 0;
    timeBreakDown_index = 0;
    timeBreakDown_join = 0;
    timeBreakDown_all = 0;timeTrackingStartNoClaim(timeBreakDown_all);
    return true;
}


bool StreamOperator::stop() {
    /**
     */
    if (lockedByWaterMark) {
        WM_INFO("early terminate by watermark, already have results");
    }
    if (!lockedByWaterMark) {
        WM_INFO("No watermark encountered, compute now");
    }
    timeBreakDown_all = timeTrackingEnd(timeBreakDown_all);
    //lazyComputeOfAQP();
    rLen = myWindow.windowR.size();
    tr = myWindow.windowR.data();
    timeNow = lastTimeOfR;
    for (size_t i = 0; i < rLen; i++) {
        if (tr[i]->arrivalTime < timeNow) { tr[i]->processedTime = timeNow; }
    }
    return true;
}

//auto StreamOperator::mswj_execution(Tuple *join_tuple) -> void {
//    Tuple tuple = *join_tuple;
//    int stream_id = tuple.streamId;
//    //计算Di
//    int delay = tuple.delay;
//
//    //计算cross-join的结果大小
//    int cross_join = 1;
//
//    if (tuple.ts >= T_op_) {
//        T_op_ = tuple.ts;
//
//        for (int i = 0; i < window_map_.size(); i++) {
//            if (window_map_[i].empty()) {
//                continue;
//            }
//            //统计window内元组数量数据
//            productivity_profiler_->add_join_record(stream_id, window_map_[i].size());
//
//            if (i == stream_id) {
//                continue;
//            }
//
//            for (auto iter = window_map_[i].begin(); iter != window_map_[i].end();) {
//                Tuple tuple_j = *iter;
//                cross_join++;
//                if (tuple_j.ts < tuple.ts - window_map_[i].size()) {
//                    window_map_[i].erase(iter++);
//                    cross_join--;
//                } else {
//                    iter++;
//                }
//            }
//
//        }
//
//        //更新cross_join_map
//
//        productivity_profiler_->update_cross_join(get_D(delay), cross_join);
//
//        //连接
//        std::unordered_map<int, std::vector<Tuple>> tempJoinMap;
//        int res = 1;
//        for (int i = 0; i < window_map_.size(); i++) {
//            if (window_map_[i].empty()) {
//                continue;
//            }
//
//            if (i == stream_id) {
//                continue;
//            }
//            while (!window_map_[i].empty()) {
//                Tuple tuple_j = window_map_[i].front();
//                window_map_[i].pop_front();
//                if (can_join_(tuple, tuple_j)) {
//                    //时间戳定义为ei.ts
//                    tuple_j.ts = tuple.ts;
//                    tempJoinMap[i].push_back(tuple_j);
//                }
//            }
//        }
//
//        int tempCount = 1;
//        //统计res,先统计二路join
//        for (const auto &it: tempJoinMap) {
//            tempCount *= it.second.size();
//            for (auto l: it.second) {
//                std::vector<Tuple> tempVec;
//                tempVec.push_back(tuple);
//                tempVec.push_back(l);
//                result_.push(tempVec);
//            }
//        }
//
//        res += tempCount;
//
//        joinResultCount_ += res;
//
//        //更新join result map
//        productivity_profiler_->update_join_res(get_D(delay), res);
//
//        window_map_[stream_id].push_back(tuple);
//    } else if (tuple.ts > T_op_ - window_map_[stream_id].size()) {
//        window_map_[stream_id].push_back(tuple);
//    }
//}


auto StreamOperator::mswj_execution(const TrackTuplePtr &trackTuple) -> bool {
    int stream_id = trackTuple->streamId;

    bool shouldGenWM;
    if (lockedByWaterMark) {
        return false;
    }

    if (stream_id == 1) {
        sIsInWindow = myWindow.feedTupleS(trackTuple);
        shouldGenWM = wmGen->reportTupleS(trackTuple, 1);
    } else {
        rIsInWindow = myWindow.feedTupleR(trackTuple);
        shouldGenWM = wmGen->reportTupleR(trackTuple, 1);
    }

    if (shouldGenWM) {
        lockedByWaterMark = true;
        WM_INFO("water mark in S");
        //  return false;
    }
    // bool shouldGenWM;
    if (stream_id == 1) {
        if (sIsInWindow) {
            productivity_profiler_->update_cross_join(get_D(trackTuple->delay),
                                                      myWindow.windowR.size() * myWindow.windowS.size());

            IMAStateOfKeyPtr sk;
            /**
             * @brief First get the index of hash table
             */
            timeTrackingStart(tt_index);
            AbstractStateOfKeyPtr skrf = stateOfKeyTableS->getByKey(trackTuple->key);
            if (skrf == nullptr) // this key does'nt exist
            {
                sk = newIMAStateOfKey();
                sk->key = trackTuple->key;
                stateOfKeyTableS->insert(sk);
            } else {
                sk = ImproveStateOfKeyTo(IMAStateOfKey, skrf);
            }
            timeBreakDown_index += timeTrackingEnd(tt_index);
            /**
             *
             */
            timeTrackingStart(tt_prediction);
            updateStateOfKey(sk, trackTuple);
            double futureTuplesS = MeanAQPIAWJOperator::predictUnarrivedTuples(sk);
            timeBreakDown_prediction += timeTrackingEnd(tt_prediction);
            //probe in R
            timeTrackingStart(tt_join);
            AbstractStateOfKeyPtr probrPtr = stateOfKeyTableR->getByKey(trackTuple->key);

            if (probrPtr != nullptr) {
                IMAStateOfKeyPtr py = ImproveStateOfKeyTo(IMAStateOfKey, probrPtr);
                confirmedResult += py->arrivedTupleCnt;
                double matchedFutureTuples = futureTuplesS * (sk->lastUnarrivedTuples + sk->arrivedTupleCnt);

                intermediateResult += matchedFutureTuples / (py->arrivedTupleCnt + futureTuplesS);
                productivity_profiler_->update_join_res(get_D(trackTuple->delay),
                                                        (futureTuplesS + sk->arrivedTupleCnt) *
                                                        (py->lastUnarrivedTuples + py->arrivedTupleCnt) -
                                                        (sk->arrivedTupleCnt + sk->lastUnarrivedTuples - 1) *
                                                        (py->lastUnarrivedTuples + py->arrivedTupleCnt));

            }
            timeBreakDown_join += timeTrackingEnd(tt_join);
            //sk->lastEstimateAllTuples=futureTuplesS+sk->arrivedTupleCnt;
            sk->pastArrivalRate =
                    trackTuple->arrivalTime - sk->lastArrivalTuple->arrivalTime == 0
                    ? 0
                    : sk->arrivedTupleCnt / (trackTuple->arrivalTime - sk->lastArrivalTuple->arrivalTime);

            sk->lastUnarrivedTuples = futureTuplesS;
            lastTimeOfR = UtilityFunctions::timeLastUs(timeBaseStruct);
        }
    } else {
        if (rIsInWindow) {
            productivity_profiler_->update_cross_join(get_D(trackTuple->delay),
                                                      myWindow.windowR.size() * myWindow.windowS.size());

            IMAStateOfKeyPtr sk;timeTrackingStart(tt_index);
            AbstractStateOfKeyPtr skrf = stateOfKeyTableR->getByKey(trackTuple->key);

            // lastTimeR=tr->arrivalTime;
            if (skrf == nullptr) // this key does'nt exist
            {
                sk = newIMAStateOfKey();
                sk->key = trackTuple->key;
                stateOfKeyTableR->insert(sk);
            } else {
                sk = ImproveStateOfKeyTo(IMAStateOfKey, skrf);
            }
            timeBreakDown_index += timeTrackingEnd(tt_index);timeTrackingStart(tt_prediction);
            updateStateOfKey(sk, trackTuple);
            double futureTuplesR = MeanAQPIAWJOperator::predictUnarrivedTuples(sk);
            timeBreakDown_prediction += timeTrackingEnd(tt_prediction);
            //probe in S
            timeTrackingStart(tt_join);
            AbstractStateOfKeyPtr probrPtr = stateOfKeyTableS->getByKey(trackTuple->key);
            if (probrPtr != nullptr) {
                IMAStateOfKeyPtr py = ImproveStateOfKeyTo(IMAStateOfKey, probrPtr);
                confirmedResult += py->arrivedTupleCnt;
                double matchedFutureTuples = futureTuplesR * (py->lastUnarrivedTuples + py->arrivedTupleCnt);

                intermediateResult += matchedFutureTuples / (sk->arrivedTupleCnt + futureTuplesR);

                productivity_profiler_->update_join_res(get_D(trackTuple->delay),
                                                        (futureTuplesR + sk->arrivedTupleCnt) *
                                                        (py->lastUnarrivedTuples + py->arrivedTupleCnt) -
                                                        (sk->arrivedTupleCnt + sk->lastUnarrivedTuples - 1) *
                                                        (py->lastUnarrivedTuples + py->arrivedTupleCnt));

            }
            timeBreakDown_join += timeTrackingEnd(tt_join);

            sk->pastArrivalRate =
                    trackTuple->arrivalTime - sk->lastArrivalTuple->arrivalTime == 0
                    ? 0
                    : sk->arrivedTupleCnt / (trackTuple->arrivalTime - sk->lastArrivalTuple->arrivalTime);


            sk->lastUnarrivedTuples = futureTuplesR;
            lastTimeOfR = UtilityFunctions::timeLastUs(timeBaseStruct);
        }
    }

    return true;
}


auto StreamOperator::setConfig(INTELLI::ConfigMapPtr cfg) -> bool {
    if (!OoOJoin::MeanAQPIAWJOperator::setConfig(cfg)) {
        return false;
    }
    std::string wmTag = config->tryString("wmTag", "arrival", true);
    WMTablePtr wmTable = newWMTable();
    wmGen = wmTable->findWM(wmTag);
    if (wmGen == nullptr) {
        INTELLI_ERROR("NO such a watermarker named [" + wmTag + "]");
        return false;
    }
    INTELLI_INFO("Using the watermarker named [" + wmTag + "]");
    return true;
}

size_t StreamOperator::getResult() {

    return confirmedResult;
}

size_t StreamOperator::getAQPResult() {
    return intermediateResult;
}




