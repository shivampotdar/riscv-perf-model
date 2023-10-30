
#pragma once

#include "sparta/ports/PortSet.hpp"
#include "sparta/ports/SignalPort.hpp"
#include "sparta/ports/DataPort.hpp"
#include "sparta/events/EventSet.hpp"
#include "sparta/events/UniqueEvent.hpp"
#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/ParameterSet.hpp"
#include "sparta/simulation/TreeNode.hpp"
#include "sparta/collection/Collectable.hpp"
#include "sparta/events/StartupEvent.hpp"
#include "sparta/ports/SyncPort.hpp"
#include "sparta/resources/Pipe.hpp"

#include "Inst.hpp"
#include "CoreTypes.hpp"
#include "FlushManager.hpp"

namespace olympia_mss
{
    class MSS : public sparta::Unit
    {
    public:
        //! Parameters for MSS model
        class MSSParameterSet : public sparta::ParameterSet
        {
        public:
            // Constructor for MSSParameterSet
            MSSParameterSet(sparta::TreeNode* n):
                sparta::ParameterSet(n)
            { }

            PARAMETER(uint32_t, mss_latency, 5, "MSS access latency")
        };

        // Constructor for MSS
        // node parameter is the node that represent the MSS and p is the MSS parameter set
        MSS(sparta::TreeNode* node, const MSSParameterSet* p);

        // name of this resource.
        static const char name[];


        ////////////////////////////////////////////////////////////////////////////////
        // Type Name/Alias Declaration
        ////////////////////////////////////////////////////////////////////////////////


    private:
        ////////////////////////////////////////////////////////////////////////////////
        // Input Ports
        ////////////////////////////////////////////////////////////////////////////////

        sparta::SyncInPort<olympia::InstPtr> in_mss_req_sync_
            {&unit_port_set_, "in_mss_req_sync", getClock()};


        ////////////////////////////////////////////////////////////////////////////////
        // Output Ports
        ////////////////////////////////////////////////////////////////////////////////

        sparta::SyncOutPort<bool> out_mss_ack_sync_
            {&unit_port_set_, "out_mss_ack_sync", getClock()};

        // sparta::SyncOutPort<bool> out_mss_ack_sync0_
        //     {&unit_port_set_, "out_mss_ack_sync0", getClock()};

        // sparta::SyncOutPort<bool> out_mss_ack_sync1_
        //     {&unit_port_set_, "out_mss_ack_sync1", getClock()};


        ////////////////////////////////////////////////////////////////////////////////
        // Internal States
        ////////////////////////////////////////////////////////////////////////////////
        const uint32_t mss_latency_;
        bool mss_busy_ = false;


        ////////////////////////////////////////////////////////////////////////////////
        // Event Handlers
        ////////////////////////////////////////////////////////////////////////////////

        // Event to handle MSS request from BIU
        sparta::UniqueEvent<> ev_handle_mss_req_
            {&unit_event_set_, "handle_mss_req", CREATE_SPARTA_HANDLER(MSS, handle_MSS_req_)};

        // sparta::UniqueEvent<> ev_handle_mss_req0_
        //     {&unit_event_set_, "handle_mss_req0", CREATE_SPARTA_HANDLER(MSS, handle_MSS_req0_)};

        // sparta::UniqueEvent<> ev_handle_mss_req1_
        //     {&unit_event_set_, "handle_mss_req1", CREATE_SPARTA_HANDLER(MSS, handle_MSS_req1_)};

        ////////////////////////////////////////////////////////////////////////////////
        // Callbacks
        ////////////////////////////////////////////////////////////////////////////////

        // Receive new MSS request from BIU
        void getReqFromBIU_(const olympia::InstPtr &);

        // Handle MSS request
        void handle_MSS_req_();
        // void handle_MSS_req0_();
        // void handle_MSS_req1_();


        ////////////////////////////////////////////////////////////////////////////////
        // Regular Function/Subroutine Call
        ////////////////////////////////////////////////////////////////////////////////


    };
}
