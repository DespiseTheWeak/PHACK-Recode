#include "..\includes.h"
#include "../UTILS/interfaces.h"
#include "../UTILS/offsets.h"
#include "../SDK/CBaseAnimState.h"
#include "../SDK/CInput.h"
#include "../SDK/IClient.h"
#include "../SDK/CPanel.h"
#include "../UTILS/render.h"
#include "../SDK/ConVar.h"
#include "../SDK/CGlowObjectManager.h"
#include "../SDK/IEngine.h"
#include "../SDK/CTrace.h"
#include "../SDK/CClientEntityList.h"
#include "../SDK/CBaseWeapon.h"
#include "../SDK/ModelInfo.h"
#include "../SDK/ModelRender.h"
#include "../SDK/RenderView.h"
#include "../SDK/CTrace.h"
#include "../SDK/CViewSetup.h"
#include "../SDK/CGlobalVars.h"
#include "../UTILS/NetvarHookManager.h"
#include "../SDK/CBaseEntity.h"
#include "../SDK/RecvData.h"


#include "../MENU/menu_framework.h"

namespace SDK
{
	class NetChannel;

}

class CIncomingSequence {
public:
	int in_reliable_state,
		out_reliable_state,
		in_sequence_nr;
	float realtime;

	CIncomingSequence();

	CIncomingSequence(int irs, int ors, int isn, float _realtime);
};

class CFakeLatency {
public:
	int SendDatagram(SDK::NetChannel* netchan, void* datagram);
	void UpdateIncomingSequences(ClientFrameStage_t curStage);
	void AddLatency(SDK::NetChannel* netchan, float latency);

	std::deque <CIncomingSequence> m_sequences;
	float lastincomingsequencenumber;

	SDK::NetChannel* m_netchan;
	int ticks = 0;
};

extern CFakeLatency* g_FakeLatency;