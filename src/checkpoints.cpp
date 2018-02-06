// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "checkpoints.h"

#include "txdb.h"
#include "main.h"
#include "uint256.h"


static const int nCheckpointSpan = 5000;

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;

    //
    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    //
    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
		(0, uint256("0x0000ae7791654eb9236eb62fe48f8796f7f8021561646783ec4e172a68a36af7"))
		(1, uint256("0x09acce2d58beaf668e21e0872f96a83af906fec8e4747a5a2f665825d668b2a1"))
		(2, uint256("0x16984499b3e37037630bc574f5c57457f7d850ef0aef0ab71cde07f83f3c82f6"))
		(3, uint256("0x6a27f0d80b3ceb0958cd11c079258c32c9782ffb9ca2364200472678e42a2b35"))
		(5, uint256("0x1ac424014092cb32c1ac71e0319a5243b1592466f7a482b1e7a774147cdef07b"))
		(10, uint256("0x75469c9f207d79019d8e87f11cf8060186029acb8f60f2bac545c66d8675679a"))
		(20, uint256("0x6d70c186b0829630902b1b71f181dcea3959f57267a8bceba42eb48491525a55"))
		(30, uint256("0xc52c19b20b962046861ba473f181083c022aeb93ec00562254d887317669953e"))
		(50, uint256("0xb14a2fa78830557a083cf6c19d21e3deb7e1face525702f2a250df1edda316d9"))
		(100, uint256("0xd4c04d81bd528646ca6ff9302dc047bb86cda4b9f8d7982e33970af9f09c837f"))
		(150, uint256("0x89e86eb29657c05d9c1f274e0500ce58b7ecb68a4131f6a8241f9b148012663c"))
		(200, uint256("0x27e17b78edcffbc1a6ca6b53c8189d30f5332978e490ad0c86bcb604c572a05e"))
		(201, uint256("0x38c5ca3d74d4e429809183e2c8048aa1cd5259b2a74f34083fc8ed205e72404c"))
		(205, uint256("0x38cefe52cf24936e6b1c995978d78d646d1f093308abb51c5c681132fbdf09e3"))
		(210, uint256("0xf760fdf1d1f4b29cabc0892bbce1d3c112730aa07f35672f9d17fb1f96382309"))
		(220, uint256("0x10786bfc3ce92847b182a154e1bc70cf7b84a36d3a76eacc4d907b8fa9afa1c4"))
		(230, uint256("0xce2791e89e68cd0eb70362498bb9ae595fdb2673b066626575fe6828b41b3365"))
		(300, uint256("0x815ca5bd2e875d24cb5ecdf37736b8e18ddf086faf070b89435511bcb5027873"))
		(600, uint256("0x98de6daf1ecb0caec0731a72bed6058b941d929055793f2c563a338cb053dc03"))
		(1200, uint256("0x5fcfb3ec33db042421a8e99416e9dde7724815aa5307028803871b340eb51e8f"))
		(1800, uint256("0x6f8794bcf80d2c6a3876c888e1f47ba5dd7419504d208cb41e0e55eefac6daa2"))
		(2400, uint256("0xa7bad7e5986f74cfbc16b1f63b17b5baf27df555d764559aaf0fe9786ea4bd35"))
		(3000, uint256("0xf1a6f34e5ce06b8080fb1e4f095344f9b4ab51772032fb2539c1507dd1a1f490"))
		(3600, uint256("0xaff6aec357d620b615a3135dd310d897e519c267826e5afa158e03a00403279c"))
		(4000, uint256("0x1eaeaea2b7fac4e0cf7312079b25dd8c3b2980f2f95137e871d30816099484df"))
		(4600, uint256("0x9eb663c4c54740ece83f29b199cbed14d2d5bd837d262287386ba6065d5c3e91"))
		(5000, uint256("0xba136935c2667fbf88f8b9b03765f4f602babcad917acd71c46c84ba58ba2081"))
		(5600, uint256("0x41868a23fec52ffcd29dcc941def9413305f37a2da323a704011fecd99650a4d"))
		(6000, uint256("0x688a4fad022ab255d540ca883d9308f8a7b7e5583fabb2392cde6b7daafa4251"))
    ;

    // TestNet has no checkpoints
    static MapCheckpoints mapCheckpointsTestnet;

    bool CheckHardened(int nHeight, const uint256& hash)
    {
        MapCheckpoints& checkpoints = (TestNet() ? mapCheckpointsTestnet : mapCheckpoints);

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    int GetTotalBlocksEstimate()
    {
        MapCheckpoints& checkpoints = (TestNet() ? mapCheckpointsTestnet : mapCheckpoints);

        if (checkpoints.empty())
            return 0;
        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        MapCheckpoints& checkpoints = (TestNet() ? mapCheckpointsTestnet : mapCheckpoints);

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }

    // Automatically select a suitable sync-checkpoint 
    const CBlockIndex* AutoSelectSyncCheckpoint()
    {
        const CBlockIndex *pindex = pindexBest;
        // Search backward for a block within max span and maturity window
        while (pindex->pprev && pindex->nHeight + nCheckpointSpan > pindexBest->nHeight)
            pindex = pindex->pprev;
        return pindex;
    }

    // Check against synchronized checkpoint
    bool CheckSync(int nHeight)
    {
        const CBlockIndex* pindexSync = AutoSelectSyncCheckpoint();
        if (nHeight <= pindexSync->nHeight){
            return false;
        }
        return true;
    }
}
