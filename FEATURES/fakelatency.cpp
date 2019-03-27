#include "fakelatency.h"
#include "../HOOKS/hooks.h"
#include "../SDK/shitstate.h"
#include "../SDK/CGlobalVars.h"
#include "../SDK/NetChannel.h"
#include "../UTILS/interfaces.h"



CIncomingSequence::CIncomingSequence()
{
	in_reliable_state = (*INTERFACES::client_state)->m_NetChannel->m_nInReliableState;
	out_reliable_state = (*INTERFACES::client_state)->m_NetChannel->m_nOutReliableState;
	in_sequence_nr = (*INTERFACES::client_state)->m_NetChannel->m_nInSequenceNr;
	realtime = INTERFACES::Globals->realtime;
}
CIncomingSequence::CIncomingSequence(int irs, int ors, int isn, float _realtime)
{
	in_reliable_state = irs;
	out_reliable_state = ors;
	in_sequence_nr = isn;
	realtime = _realtime;
}



void CFakeLatency::AddLatency(SDK::NetChannel * netchan, float latency) {
	for (auto& seq : m_sequences) {
		if (INTERFACES::Globals->realtime - seq.realtime >= latency) {
			netchan->m_nInReliableState = seq.in_reliable_state;
			netchan->m_nInSequenceNr = seq.in_sequence_nr;
			break;
		}
	}
}

void CFakeLatency::UpdateIncomingSequences(ClientFrameStage_t curStage) {
	if (curStage != FRAME_NET_UPDATE_POSTDATAUPDATE_START)
		return;

	if (!(*INTERFACES::client_state) || !(*INTERFACES::client_state)->m_NetChannel)
		return;

	if ((*INTERFACES::client_state)->m_NetChannel->m_nInSequenceNr > lastincomingsequencenumber) {
		lastincomingsequencenumber = (*INTERFACES::client_state)->m_NetChannel->m_nInSequenceNr;

		m_sequences.push_front(CIncomingSequence((*INTERFACES::client_state)->m_NetChannel->m_nInReliableState, (*INTERFACES::client_state)->m_NetChannel->m_nOutReliableState, (*INTERFACES::client_state)->m_NetChannel->m_nInSequenceNr, INTERFACES::Globals->realtime));
	}

	if (m_sequences.size() > 2048)
		m_sequences.pop_back();
}
int CFakeLatency::SendDatagram(SDK::NetChannel* netchan, void* datagram) {
	if (!INTERFACES::Engine->IsInGame() || !INTERFACES::Engine->IsConnected() || datagram)
		return HOOKS::original_send_datagram(netchan, datagram);


	if (!SETTINGS::settings.fakelatency_enabled)
		return HOOKS::original_send_datagram(netchan, datagram);

	auto instate = netchan->m_nInReliableState;
	auto in_sequencenr = netchan->m_nInSequenceNr;

	AddLatency(netchan, SETTINGS::settings.fakelatency_amount);

	int ret = HOOKS::original_send_datagram(netchan, datagram);

	netchan->m_nInReliableState = instate;
	netchan->m_nInSequenceNr = in_sequencenr;

	return ret;
}

CFakeLatency* g_FakeLatency = new CFakeLatency;