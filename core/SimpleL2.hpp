
#pragma once

#include "sparta/utils/SpartaAssert.hpp"
#include "sparta/utils/MathUtils.hpp"
#include "cache/BasicCacheItem.hpp"
#include "cache/SimpleCache2.hpp"
#include "cache/ReplacementIF.hpp"
#include "cache/preload/PreloadableIF.hpp"
#include "cache/preload/PreloadableNode.hpp"
#include "CacheFuncModel.hpp"
using namespace std::placeholders;
namespace olympia
{
    
    class SimpleL2: public sparta::cache::SimpleCache2<SimpleCacheLine>,
                      public sparta::TreeNode,
                      public sparta::cache::PreloadableIF,
                      public sparta::cache::PreloadDumpableIF

    {
    public:
        using Handle = std::shared_ptr<SimpleL2>;
        SimpleL2(sparta::TreeNode* parent,
                  uint64_t cache_size_kb,
                  uint64_t line_size,
                  const sparta::cache::ReplacementIF& rep) :
            sparta::cache::SimpleCache2<SimpleCacheLine> (cache_size_kb,
                                                        line_size,
                                                        line_size,
                                                        SimpleCacheLine(line_size),
                                                        rep),
            sparta::TreeNode(parent, "l1cache", "Simple L1 DCache"),
            sparta::cache::PreloadableIF(),
            sparta::cache::PreloadDumpableIF(),
            preloadable_(this, std::bind(&SimpleL2::preloadPkt_, this, _1),
                         std::bind(&SimpleL2::preloadDump_, this, _1))
        {}
    private:
        /**
         * Implement a preload by just doing a fill to the va in the packet.
         */
        bool preloadPkt_(sparta::cache::PreloadPkt& pkt) override
        {
            sparta::cache::PreloadPkt::NodeList lines;
            pkt.getList(lines);
            for (auto& line_data : lines)
            {
                uint64_t va = line_data->getScalar<uint64_t>("va");
                auto& cache_line = getLineForReplacement(va);
                std::cout << *this << " : Preloading VA: 0x" << std::hex << va
                          << std::endl;
                allocateWithMRUUpdate(cache_line, va);
                // Sanity check that the line was marked as valid.
                sparta_assert(getLine(va) != nullptr);
            }
            return true;

        }

        void preloadDump_(sparta::cache::PreloadEmitter& emitter) const override
        {
            emitter << sparta::cache::PreloadEmitter::BeginMap;
            emitter << sparta::cache::PreloadEmitter::Key << "lines";
            emitter << sparta::cache::PreloadEmitter::Value;
            emitter << sparta::cache::PreloadEmitter::BeginSeq;
            for (auto set_it = begin();
                 set_it != end(); ++set_it)
            {
                for (auto line_it = set_it->begin();
                     line_it != set_it->end(); ++line_it)
                {
                    if(line_it->isValid())
                    {
                        std::map<std::string, std::string> map;
                        std::stringstream t;
                        t << "0x" << std::hex << line_it->getAddr();
                        map["pa"] = t.str();
                        emitter << map;
                    }
                }
            }
            emitter << sparta::cache::PreloadEmitter::EndSeq;
            emitter << sparta::cache::PreloadEmitter::EndMap;
        }
        //! Provide a preloadable node that hangs off and just returns
        //! the preloadPkt call to us.
        sparta::cache::PreloadableNode preloadable_;

    }; // class SimpleDL1

} // namespace olympia
