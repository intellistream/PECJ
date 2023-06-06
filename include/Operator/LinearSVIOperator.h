/*! \file   LinearSVIOperator.h
.h*/
//
// Created by tony on 03/12/22.
//

#ifndef INTELLISTREAM_INCLUDE_OPERATOR_LinearSVIOperator_H_
#define INTELLISTREAM_INCLUDE_OPERATOR_LinearSVIOperator_H_

#include <Operator/MeanAQPIAWJOperator.h>
#include <Common/Window.h>
#include <atomic>
#include <WaterMarker/LatenessWM.h>
#include <Common/StateOfKey.h>
#include <Common/LinearSVI.h>
#include <optional>
#include <filesystem>
#include <Common/ObservationGroup.hpp>
using namespace OBSERVAYION_GROUP;
using std::nullopt;
namespace OoOJoin {

/**
 * @ingroup ADB_OPERATORS
 * @class LinearSVIOperator Operator/LinearSVIOperator.h
 * @brief The IAWJ operator working under LinearSVI and use variational inference to proactively capture the unobserved
 * @note require configurations:
 * - "windowLen" U64: The length of window
 * - "slideLen" U64: The length of slide
 * - "sLen" U64: The length of S buffer
 * - "rLen" U64: The length of R buffer
 * - "wmTag" String: The tag of watermarker, default is arrival for @ref ArrivalWM
 * - "aiMode" String: The tag to indicate working mode of ai, can be pretrain (0), continual_learning (1) or inference (2)
 * = "ptPrefix" String: The prefix of svi *.pt, such as linearVAE
 * - "appendSel", U64, whether append sel observations to stored tensor, 0
 * - "appendSkew", U64, whether append skew (of r and s) observations to stored tensor, 0
 * - "appendRate", U64, whether append rate (of r and s) observations to stored tensor, 0
 * - "exitAfterPretrain", U64, whether exit after a pretrain, U64,1
 * @warning This implementation is putting rotten, just to explore a basic idea of AQP by using historical mean to predict future
 * @warning The predictor and watermarker are currently NOT seperated in this operator, split them in the future!
 * @note In current version, the computation will block feeding
 * @note follows the assumption of linear independent arrival and skewness
 * @note operator tag = "LinearSVI"
 */
class LinearSVIOperator : public MeanAQPIAWJOperator {
 protected:
  void conductComputation();
  std::string aiMode;
  uint8_t aiModeEnum = 0;
  uint64_t appendSel = 0, appendSkew = 0, appendRate, exitAfterPretrain;
  std::string ptPrefix;
  /**
   * @brief The pre-allocated length of seletivity observations, only valid for pretrain
   */
  uint64_t selLen = 0;

  class LinearSVIStateOfKey : public MeanStateOfKey {
   public:
    double lastUnarrivedTuples = 0;

    LinearSVIStateOfKey() = default;

    ~LinearSVIStateOfKey() = default;
  };
  class LinearSVIStateOfStreams {
   public:
    uint64_t sCnt = 0, rCnt = 0;
    /**
     * @brief estimate and track the selectivity
     */
    ObservationGroup selObservations;
    /**
     * @brief estimate and track the skew of s and r
     */
    ObservationGroup sSkewObservations, rSkewObservations;
    /**
     * @brief estimate and track the rate of s and r
     */
    ObservationGroup sRateObservations, rRateObservations;
    double selectivity = 0.0;

    uint64_t sEventTime = 0, rEventTime = 0;
    double sRate = 0, rRate = 0;
    double sSkew = 0, rSkew = 0;
    void updateSelectivity(uint64_t joinResults) {
      double crossCnt = rCnt * sCnt;
      selectivity = joinResults / crossCnt;

    }
    void encounterSTuple(TrackTuplePtr ts) {
      sCnt++;
      if (ts->eventTime > sEventTime) {
        sEventTime = ts->eventTime;
      }
      sRate = sCnt;
      sRate = sRate * 1e3 / sEventTime;
      sSkew = (sSkew * (sCnt - 1) + ts->arrivalTime - ts->eventTime) / sCnt;
      //sSkewObservations.appendX(sSkew);
      // sRateObservations.appendX(sRate);
    }
    void encounterRTuple(TrackTuplePtr tr) {
      rCnt++;
      if (tr->eventTime > rEventTime) {
        rEventTime = tr->eventTime;
      }
      rRate = rCnt;
      rRate = rRate * 1e3 / rEventTime;
      rSkew = (rSkew * (rCnt - 1) + tr->arrivalTime - tr->eventTime) / rCnt;
      // rSkewObservations.appendX(rSkew);
      // rRateObservations.appendX(rRate);
    }
    void reset() {
      sCnt = 0;
      rCnt = 0;
      selectivity = 0.0;
      sEventTime = 0;
      rEventTime = 0;
      sRate = 0;
      rRate = 0;
      sSkew = 0;
      rSkew = 0;
    }
    std::string reportStr() {
      std::string ru = "sRate," + to_string(sRate) + "\r\n";
      ru += "rRate," + to_string(rRate) + "\r\n";
      ru += "sSkew," + to_string(sSkew) + "\r\n";
      ru += "rSkew," + to_string(rSkew) + "\r\n";
      ru += "selectivity," + to_string(selectivity) + "\r\n";
      ru += "sCnt," + to_string(sCnt) + "\r\n";
      ru += "rCnt," + to_string(rCnt) + "\r\n";
      return ru;
    }
    TROCHPACK_SVI::LinearSVI sviSelectivity, sviSRate, sviRRate;
    LinearSVIStateOfStreams() = default;
    ~LinearSVIStateOfStreams() = default;
  };
  LinearSVIStateOfStreams streamStatisics;
  void updateEstimation(ObservationGroup &observation, TROCHPACK_SVI::LinearSVI &estimator);
#define newLinearSVIStateOfKey std::make_shared<LinearSVIStateOfKey>
  using LinearSVIStateOfKeyPtr = std::shared_ptr<LinearSVIStateOfKey>;
  /**
   * @brief inline function to be called at the end of window
   */
  void endOfWindow();
  /**
   * @brief to prepare data structures for a pretrain
   */
  void preparePretrain();
  /**
 * @brief to prepare data structures for a inference
 */
  void prepareInference();
  /**
   * @brief save all tensors to file
   */
  void saveAllTensors();

 public:
  LinearSVIOperator() = default;

  ~LinearSVIOperator() = default;

  /**
   *
  * @brief Set the config map related to this operator
  * @param cfg The config map
   * @return bool whether the config is successfully set
  */
  bool setConfig(ConfigMapPtr cfg) override;

  /**
 * @brief feed a tuple s into the Operator
 * @param ts The tuple
  * @warning The current version is simplified and assuming only used in SINGLE THREAD!
  * @return bool, whether tuple is fed.
 */
  bool feedTupleS(TrackTuplePtr ts) override;

  /**
    * @brief feed a tuple R into the Operator
    * @param tr The tuple
    * @warning The current version is simplified and assuming only used in SINGLE THREAD!
    *  @return bool, whether tuple is fed.
    */
  bool feedTupleR(TrackTuplePtr tr) override;

  /**
   * @brief start this operator
   * @return bool, whether start successfully
   */
  bool start() override;

  /**
   * @brief stop this operator
   * @return bool, whether start successfully
   */
  bool stop() override;

  /**
   * @brief get the joined sum result
   * @return The result
   */
  size_t getResult() override;

  /**
   * @brief get the joined sum result under AQP
   * @return The result
   */
  size_t getAQPResult() override;

};

/**
 * @ingroup ADB_OPERATORS
 * @typedef LinearSVIOperatorPtr
 * @brief The class to describe a shared pointer to @ref LinearSVIOperator

 */
typedef std::shared_ptr<class LinearSVIOperator> LinearSVIOperatorPtr;
/**
 * @ingroup ADB_OPERATORS
 * @def newLinearSVIOperator

 * @brief (Macro) To creat a new @ref LinearSVIOperator
 under shared pointer.
 */
#define newLinearSVIOperator std::make_shared<OoOJoin::LinearSVIOperator>

}
#endif //INTELLISTREAM_INCLUDE_OPERATOR_MEANAQPLinearSVIOperator_H_