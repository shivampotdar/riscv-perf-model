#pragma once

#include "sparta/simulation/Unit.hpp"
#include "sparta/ports/DataPort.hpp"
#include "sparta/ports/SignalPort.hpp"
#include "sparta/simulation/ParameterSet.hpp"
#include "sparta/utils/LogUtils.hpp"
#include "SimpleL2.hpp"
#include "Inst.hpp"
#include "cache/TreePLRUReplacement.hpp"
#include "MemoryAccessInfo.hpp"

namespace olympia {
    class L2Cache : public sparta::Unit {
    public:
        class CacheParameterSet : public sparta::ParameterSet {
        public:
            CacheParameterSet(sparta::TreeNode *n) : sparta::ParameterSet(n) {}

            // Parameters for the L2 cache
            PARAMETER(uint32_t, l2_line_size, 64, "L2 line size (power of 2)")
            PARAMETER(uint32_t, l2_size_kb, 256, "Size of L2 in KB (power of 2)")
            PARAMETER(uint32_t, l2_associativity, 8, "L2 associativity (power of 2)")
            PARAMETER(uint32_t, cache_latency, 1, "Assumed latency of the memory system")
            PARAMETER(bool, l2_always_hit, false, "L2 will always hit")
        };

        static const char name[];
        L2Cache(sparta::TreeNode *n, const CacheParameterSet *p);

    private:
        bool dataLookup_(const MemoryAccessInfoPtr &mem_access_info_ptr);

        void reloadCache_(uint64_t phy_addr);

        void getInstsFromDL1_(const MemoryAccessInfoPtr &memory_access_info_ptr);

        void getAckFromBIU_(const InstPtr &inst_ptr);

        using L2Handle = SimpleL2::Handle;
        L2Handle l2_cache_;
        const bool l2_always_hit_;
        bool busy_;
        uint32_t cache_latency_;
        // Keep track of the instruction that causes current outstanding cache miss
        MemoryAccessInfoPtr cache_pending_inst_ = nullptr;

        ////////////////////////////////////////////////////////////////////////////////
        // Input Ports
        ////////////////////////////////////////////////////////////////////////////////
        sparta::DataInPort<MemoryAccessInfoPtr> in_dl1_lookup_req_
                {&unit_port_set_, "in_dl1_lookup_req", 0};

        sparta::DataInPort<InstPtr> in_biu_ack_
                {&unit_port_set_, "in_biu_ack", 1};

        ////////////////////////////////////////////////////////////////////////////////
        // Output Ports
        ////////////////////////////////////////////////////////////////////////////////

        sparta::DataOutPort<MemoryAccessInfoPtr> out_dl1_lookup_ack_
                {&unit_port_set_, "out_dl1_lookup_ack", 0};

        sparta::DataOutPort<InstPtr> out_biu_req_
                {&unit_port_set_, "out_biu_req"};

        ////////////////////////////////////////////////////////////////////////////////
        // Events
        ////////////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////////////
        // Counters
        ////////////////////////////////////////////////////////////////////////////////
        sparta::Counter l2_cache_hits_{
                getStatisticSet(), "l2_cache_hits",
                "Number of L2 cache hits", sparta::Counter::COUNT_NORMAL
        };

        sparta::Counter l2_cache_misses_{
                getStatisticSet(), "l2_cache_misses",
                "Number of L2 cache misses", sparta::Counter::COUNT_NORMAL
        };
    };

}
