#pragma once

class bf_read {
public:
	const char* m_pDebugName;
	bool m_bOverflow;
	int m_nDataBits;
	unsigned int m_nDataBytes;
	unsigned int m_nInBufWord;
	int m_nBitsAvail;
	const unsigned int* m_pDataIn;
	const unsigned int* m_pBufferEnd;
	const unsigned int* m_pData;

	bf_read() = default;

	bf_read(const void* pData, int nBytes, int nBits = -1) {
		StartReading(pData, nBytes, 0, nBits);
	}

	void StartReading(const void* pData, int nBytes, int iStartBit, int nBits) {
		// Make sure it's dword aligned and padded.
		m_pData = (uint32_t*)pData;
		m_pDataIn = m_pData;
		m_nDataBytes = nBytes;

		if (nBits == -1) {
			m_nDataBits = nBytes << 3;
		}
		else {
			m_nDataBits = nBits;
		}
		m_bOverflow = false;
		m_pBufferEnd = reinterpret_cast<uint32_t const*>(reinterpret_cast<uint8_t const*>(m_pData) + nBytes);
		if (m_pData)
			Seek(iStartBit);
	}

	bool Seek(int nPosition) {
		bool bSucc = true;
		if (nPosition < 0 || nPosition > m_nDataBits) {
			m_bOverflow = true;
			bSucc = false;
			nPosition = m_nDataBits;
		}
		int nHead = m_nDataBytes & 3; // non-multiple-of-4 bytes at head of buffer. We put the "round off"
									  // at the head to make reading and detecting the end efficient.

		int nByteOfs = nPosition / 8;
		if ((m_nDataBytes < 4) || (nHead && (nByteOfs < nHead))) {
			// partial first dword
			uint8_t const* pPartial = (uint8_t const*)m_pData;
			if (m_pData) {
				m_nInBufWord = *(pPartial++);
				if (nHead > 1)
					m_nInBufWord |= (*pPartial++) << 8;
				if (nHead > 2)
					m_nInBufWord |= (*pPartial++) << 16;
			}
			m_pDataIn = (uint32_t const*)pPartial;
			m_nInBufWord >>= (nPosition & 31);
			m_nBitsAvail = (nHead << 3) - (nPosition & 31);
		}
		else {
			int nAdjPosition = nPosition - (nHead << 3);
			m_pDataIn = reinterpret_cast<uint32_t const*>(
				reinterpret_cast<uint8_t const*>(m_pData) + ((nAdjPosition / 32) << 2) + nHead);
			if (m_pData) {
				m_nBitsAvail = 32;
				GrabNextDWord();
			}
			else {
				m_nInBufWord = 0;
				m_nBitsAvail = 1;
			}
			m_nInBufWord >>= (nAdjPosition & 31);
			m_nBitsAvail = min(m_nBitsAvail, 32 - (nAdjPosition & 31)); // in case grabnextdword overflowed
		}
		return bSucc;
	}

	FORCEINLINE void GrabNextDWord(bool bOverFlowImmediately = false) {
		if (m_pDataIn == m_pBufferEnd) {
			m_nBitsAvail = 1; // so that next read will run out of words
			m_nInBufWord = 0;
			m_pDataIn++; // so seek count increments like old
			if (bOverFlowImmediately)
				m_bOverflow = true;
		}
		else if (m_pDataIn > m_pBufferEnd) {
			m_bOverflow = true;
			m_nInBufWord = 0;
		}
		else {
			m_nInBufWord = DWORD(*(m_pDataIn++));
		}
	}
};


class bf_write {
public:
	unsigned char* m_pData;
	int m_nDataBytes;
	int m_nDataBits;
	int m_iCurBit;
	bool m_bOverflow;
	bool m_bAssertOnOverflow;
	const char* m_pDebugName;

	void StartWriting(void* pData, int nBytes, int iStartBit = 0, int nBits = -1) {
		// Make sure it's dword aligned and padded.
		// The writing code will overrun the end of the buffer if it isn't dword aligned, so truncate to force alignment
		nBytes &= ~3;

		m_pData = (unsigned char*)pData;
		m_nDataBytes = nBytes;

		if (nBits == -1) {
			m_nDataBits = nBytes << 3;
		}
		else {
			m_nDataBits = nBits;
		}

		m_iCurBit = iStartBit;
		m_bOverflow = false;
	}

	bf_write() {
		m_pData = NULL;
		m_nDataBytes = 0;
		m_nDataBits = -1; // set to -1 so we generate overflow on any operation
		m_iCurBit = 0;
		m_bOverflow = false;
		m_bAssertOnOverflow = true;
		m_pDebugName = NULL;
	}

	// nMaxBits can be used as the number of bits in the buffer.
	// It must be <= nBytes*8. If you leave it at -1, then it's set to nBytes * 8.
	bf_write(void* pData, int nBytes, int nBits = -1) {
		m_bAssertOnOverflow = true;
		m_pDebugName = NULL;
		StartWriting(pData, nBytes, 0, nBits);
	}

	bf_write(const char* pDebugName, void* pData, int nBytes, int nBits = -1) {
		m_bAssertOnOverflow = true;
		m_pDebugName = pDebugName;
		StartWriting(pData, nBytes, 0, nBits);
	}
};
typedef enum
{
	NA_NULL = 0,
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP,
} netadrtype_t;
typedef unsigned int uint;
typedef unsigned char uint8;
typedef unsigned __int16		uint16;
typedef struct netadr_s
{
public:
	netadr_s() { SetIP(0); SetPort(0); SetType(NA_IP); }
	netadr_s(uint unIP, uint16 usPort) { SetIP(unIP); SetPort(usPort); SetType(NA_IP); }
	netadr_s(const char* pch) { SetFromString(pch); }
	void	Clear();	// invalids Address

	void	SetType(netadrtype_t type);
	void	SetPort(unsigned short port);
	bool	SetFromSockadr(const struct sockaddr* s);
	void	SetIP(uint8 b1, uint8 b2, uint8 b3, uint8 b4);
	void	SetIP(uint unIP);									// Sets IP.  unIP is in host order (little-endian)
	void    SetIPAndPort(uint unIP, unsigned short usPort) { SetIP(unIP); SetPort(usPort); }
	void	SetFromString(const char* pch, bool bUseDNS = false); // if bUseDNS is true then do a DNS lookup if needed

	bool	CompareAdr(const netadr_s& a, bool onlyBase = false) const;
	bool	CompareClassBAdr(const netadr_s& a) const;
	bool	CompareClassCAdr(const netadr_s& a) const;

	netadrtype_t	GetType() const;
	unsigned short	GetPort() const;
	const char* ToString(bool onlyBase = false) const; // returns xxx.xxx.xxx.xxx:ppppp
	void			ToSockadr(struct sockaddr* s) const;
	unsigned int	GetIPHostByteOrder() const;
	unsigned int	GetIPNetworkByteOrder() const;
	unsigned int GetIP() const;
	unsigned long addr_ntohl() const;
	unsigned long addr_htonl() const;
	bool	IsLocalhost() const; // true, if this is the localhost IP 
	bool	IsLoopback() const;	// true if engine loopback buffers are used
	bool	IsReservedAdr() const; // true, if this is a private LAN IP
	bool	IsValid() const;	// ip & port != 0
	void    SetFromSocket(int hSocket);
	bool operator==(const netadr_s& netadr) const { return (CompareAdr(netadr)); }
	bool operator<(const netadr_s& netadr) const;

public:	// members are public to avoid to much changes

	netadrtype_t	type;
	unsigned char	ip[4];
	unsigned short	port;
} netadr_t;

#define MAX_FLOWS 2
#define MAX_STREAMS 2
#define MAX_SUBCHANNELS		8		// we have 8 alternative send&wait bits
#define SUBCHANNEL_FREE		0	// subchannel is free to use
#define SUBCHANNEL_TOSEND	1	// subchannel has data, but not send yet
#define SUBCHANNEL_WAITING	2   // sbuchannel sent data, waiting for ACK
#define SUBCHANNEL_DIRTY	3	// subchannel is marked as dirty during changelevel

#define FRAGMENT_BITS		8
#define FRAGMENT_SIZE		(1<<FRAGMENT_BITS)
#define BYTES2FRAGMENTS(i) ((i+FRAGMENT_SIZE-1)/FRAGMENT_SIZE)
#define NET_MAX_PAYLOAD_BITS 19
#define MAX_FILE_SIZE_BITS 26
#define MAX_FILE_SIZE		((1<<MAX_FILE_SIZE_BITS)-1)	// maximum transferable size is	64MB

#define	FRAG_NORMAL_STREAM	0
#define FRAG_FILE_STREAM	1
#define	MAX_OSPATH		260
typedef struct dataFragments_s
{
	DWORD			file;			// open file handle
	char			filename[MAX_OSPATH]; // filename
	char* buffer;			// if NULL it's a file
	unsigned int	bytes;			// size in bytes
	unsigned int	bits;			// size in bits
	unsigned int	transferID;		// only for files
	bool			isCompressed;	// true if data is bzip compressed
	unsigned int	nUncompressedSize; // full size in bytes
	//bool			asTCP;			// send as TCP stream
	bool			isReplayDemo;	// if it's a file, is it a replay .dem file?
	int				numFragments;	// number of total fragments
	int				ackedFragments; // number of fragments send & acknowledged
	int				pendingFragments; // number of fragments send, but not acknowledged yet
} dataFragments_t;

struct subChannel_s
{
	int				startFraggment[MAX_STREAMS];
	int				numFragments[MAX_STREAMS];
	int				sendSeqNr;
	int				state; // 0 = free, 1 = scheduled to send, 2 = send & waiting, 3 = dirty
	int				index; // index in m_SubChannels[]

	void Free()
	{
		state = SUBCHANNEL_FREE;
		sendSeqNr = -1;
		for (int i = 0; i < MAX_STREAMS; i++)
		{
			numFragments[i] = 0;
			startFraggment[i] = -1;
		}
	}
};
struct netframe_t_firstpart
{
	float time; //0
	int size; //4
	__int16 choked; //8
	bool valid; //10
	char pad; //11
	float latency; //12
};
struct netframe_t_secondpart
{
	int dropped; //16
	float avg_latency;
	float m_flInterpolationAmount;
	unsigned __int16 msggroups[16];
};
struct netflow_t
{
	float nextcompute; //0
	float avgbytespersec; //4
	float avgpacketspersec; //8
	float avgloss; //12
	float avgchoke; //16
	float avglatency; //20
	float latency; //24
	int totalpackets; //28
	int totalbytes; //32
	int currentindex; //36
	netframe_t_firstpart frames[128]; //40
	netframe_t_secondpart frames2[128]; //2088
	netframe_t_firstpart* currentframe; //7720
};
class C_NetChannel;
class INetChannelHandler
{
public:
	virtual	~INetChannelHandler(void) {};

	virtual void ConnectionStart(C_NetChannel* chan) = 0;	// called first time network channel is established

	virtual void ConnectionClosing(const char* reason) = 0; // network channel is being closed by remote site

	virtual void ConnectionCrashed(const char* reason) = 0; // network error occured

	virtual void PacketStart(int incoming_sequence, int outgoing_acknowledged) = 0;	// called each time a new packet arrived

	virtual void PacketEnd(void) = 0; // all messages has been parsed

	virtual void FileRequested(const char* fileName, unsigned int transferID) = 0; // other side request a file for download

	virtual void FileReceived(const char* fileName, unsigned int transferID) = 0; // we received a file

	virtual void FileDenied(const char* fileName, unsigned int transferID) = 0;	// a file request was denied by other side
};

class C_NetChannel
{
public:
	char pad_0000[20];           //0x0000
	bool m_bProcessingMessages;  //0x0014
	bool m_bShouldDelete;        //0x0015
	char pad_0016[2];            //0x0016
	int32_t m_iOutSequenceNr;    //0x0018 last send outgoing sequence number
	int32_t m_iInSequenceNr;     //0x001C last received incoming sequnec number
	int32_t m_iOutSequenceNrAck; //0x0020 last received acknowledge outgoing sequnce number
	int32_t m_iOutReliableState; //0x0024 state of outgoing reliable data (0/1) flip flop used for loss detection
	int32_t m_iInReliableState;  //0x0028 state of incoming reliable data
	int32_t m_iChokedCommands;   //0x002C number of choked packets
	char pad_0030[1044];         //0x0030

	bf_write m_StreamReliable; //0x0030 
	/*CUtlMemory*/ char m_ReliableDataBuffer[12]; //0x0048 
	bf_write m_StreamUnreliable; //0x0054 
	/*CUtlMemory*/ char m_UnreliableDataBuffer[12]; //0x006C 
	bf_write m_StreamVoice; //0x0078 
	/*CUtlMemory*/char m_VoiceDataBuffer[12]; //0x0090 
	__int32 m_Socket; //0x009C 
	//__int32 m_StreamSocket; //0x00A0 
	__int32 m_MaxReliablePayloadSize; //0x00A4  //0x00A0 2020
	char pad_0x00A8[0x4]; //0x00A8
	netadr_t remote_address; //0x00AC //0xA8 2020
	char dylanpadding[88]; //padding added by dylan
	float last_received; //2018 0x10C
	//float last_received_pad;
	//char pad_0x00BC[0x4]; //0x00BC
	double /*float*/ connect_time; //0x00C0 //dylan found 0x110
	//char pad_0x00C4[0x4]; //0x00C4
	__int32 m_Rate;
	__int32 m_RatePad;
	double m_fClearTime; //0x128 not anymore
	CUtlVector<dataFragments_t*>	m_WaitingList[2]; //0x128 as of 2020
	char pad_blehch[0x260]; //0x150
	subChannel_s					m_SubChannels[MAX_SUBCHANNELS]; //0x3B0 as of 2020
	char pad_blech2[8]; //0x490 as of 2020

//#if ENGINE_BUILD_VERSION >= 13635
	char NEWPAD2018[4];
	//#endif
	netflow_t m_DataFlow[MAX_FLOWS]; //new 2018 0x49C //0x5C0
	int	m_MsgStats[16];	// total bytes for each message group
	__int32 m_PacketDrop; //0x4220  //dylan found 0x4250 new 0x4258 newnew 0x425C
	//char m_UnkPad[4];
	char m_Name[32]; //0x4224 
	__int32 m_ChallengeNr; //0x4244 
	float m_Timeout; //0x4280
	INetChannelHandler* m_MessageHandler; //0x4284   0x4288
	/*CUtlVector*/char m_NetMessages[16]; //dylan found 0x4284
	__int32 dylanUnknown;
	void* m_pDemoRecorder; //0x429C
	//__int32 dylanUnknown923874;
	//__int32 m_nQueuedPackets; //0x4268  //dylan found 0x4298
	float m_flInterpolationAmount; //0x42A0
	double m_flRemoteFrameTime; //0x42A4
	float m_flRemoteFrameTimeStdDeviation; //0x42AC
	__int32 m_nMaxRoutablePayloadSize; //0x42B0
	__int32 m_nSplitPacketSequence; //dylan found 0x42b4
	char pad_0x4280[0x14]; //0x4280

	PushVirtual(SendDatagram(LPVOID Data), 46, int(__thiscall*)(LPVOID, LPVOID), Data);
}; //Size: 0x0444