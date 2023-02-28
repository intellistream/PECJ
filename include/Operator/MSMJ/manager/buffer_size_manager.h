//
// Created by 86183 on 2023/1/5.
//

#ifndef DISORDERHANDLINGSYSTEM_BUFFER_SIZE_MANAGER_H
#define DISORDERHANDLINGSYSTEM_BUFFER_SIZE_MANAGER_H


#include "statistics_manager.h"
#include "Operator/MSMJ/profiler/tuple_productivity_profiler.h"

namespace MSMJ {

    class BufferSizeManager {
    public:

        explicit BufferSizeManager(StatisticsManager *statistics_manager, TupleProductivityProfiler *profiler);

        ~BufferSizeManager() = default;

        //自适应K值算法
        auto k_search(int stream_id) -> int;

        auto setConfig(INTELLI::ConfigMapPtr config) -> void;

    private:

        //论文中的函数γ(L,T)
        auto y(int K) -> double;

        INTELLI::ConfigMapPtr cfg = nullptr;

        //数据统计器
        StatisticsManager *statistics_manager_;

        //元组生产力
        TupleProductivityProfiler *productivity_profiler_;

    };

}

#endif //DISORDERHANDLINGSYSTEM_BUFFER_SIZE_MANAGER_H
