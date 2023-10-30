#include "L2Cache.hpp"

namespace olympia {
    const char L2Cache::name[] = "l2cache";

    L2Cache::L2Cache(sparta::TreeNode *n, const CacheParameterSet *p) :
            sparta::Unit(n),
            l2_always_hit_(p->l2_always_hit),
            cache_latency_(p->cache_latency){

        in_dl1_lookup_req_.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(L2Cache, getInstsFromDL1_, MemoryAccessInfoPtr));

        in_biu_ack_.registerConsumerHandler
            (CREATE_SPARTA_HANDLER_WITH_DATA(L2Cache, getAckFromBIU_, InstPtr));

        // L2 cache config
        const uint32_t l2_line_size = p->l2_line_size;
        const uint32_t l2_size_kb = p->l2_size_kb;
        const uint32_t l2_associativity = p->l2_associativity;
        std::unique_ptr<sparta::cache::ReplacementIF> repl(new sparta::cache::TreePLRUReplacement
                                                                   (l2_associativity));
        l2_cache_.reset(new SimpleL2(getContainer(), l2_size_kb, l2_line_size, *repl));
    }

    // Reload cache line
    void L2Cache::reloadCache_(uint64_t phy_addr)
    {
        auto l2_cache_line = &l2_cache_->getLineForReplacementWithInvalidCheck(phy_addr);
        l2_cache_->allocateWithMRUUpdate(*l2_cache_line, phy_addr);

        ILOG("L2 Cache reload complete!");
    }

    // Access Cache
    bool L2Cache::dataLookup_(const MemoryAccessInfoPtr & mem_access_info_ptr)
    {
        const InstPtr & inst_ptr = mem_access_info_ptr->getInstPtr();
        uint64_t phyAddr = inst_ptr->getRAdr();

        bool cache_hit = false;

        if (l2_always_hit_) {
            cache_hit = true;
        }
        else {
            auto cache_line = l2_cache_->peekLine(phyAddr);
            cache_hit = (cache_line != nullptr) && cache_line->isValid();

            // Update MRU replacement state if Cache HIT
            if (cache_hit) {
                l2_cache_->touchMRU(*cache_line);
            }
        }

        if (l2_always_hit_) {
            ILOG("L2 Cache HIT all the time: phyAddr=0x" << std::hex << phyAddr);
            l2_cache_hits_++;
        }
        else if (cache_hit) {
            ILOG("L2 Cache HIT: phyAddr=0x" << std::hex << phyAddr);
            l2_cache_hits_++;
        }
        else {
            ILOG("L2 Cache MISS: phyAddr=0x" << std::hex << phyAddr);
            l2_cache_misses_++;
        }

        return cache_hit;
    }

    void L2Cache::getInstsFromDL1_(const MemoryAccessInfoPtr &memory_access_info_ptr){
        const bool hit = dataLookup_(memory_access_info_ptr);
        if(hit){
            memory_access_info_ptr->setCacheState(MemoryAccessInfo::CacheState::HIT);
        }else{
            memory_access_info_ptr->setCacheState(MemoryAccessInfo::CacheState::MISS);
            if(!busy_) {
                busy_ = true;
                cache_pending_inst_ = memory_access_info_ptr;
                out_biu_req_.send(cache_pending_inst_->getInstPtr());
            }
        }
        out_dl1_lookup_ack_.send(memory_access_info_ptr);
    }

    void L2Cache::getAckFromBIU_(const InstPtr &inst_ptr) {
        reloadCache_(inst_ptr->getRAdr());
        cache_pending_inst_.reset();
        busy_ = false;
    }

}
