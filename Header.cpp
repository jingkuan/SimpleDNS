#include "DnsServer.h"
#include <time.h>

dns::Header::Header()
: m_id(0)
, m_qdcount(0)
, m_ancount(0)
, m_nscount(0)
, m_arcount(0)
{
    memset(&m_flags, 0, sizeof(m_flags));
    srand((unsigned int)clock());
}

dns::Header::~Header()
{
    
}

// Set ID or create a random ID
// todo deprecate random
unsigned short dns::Header::idset(unsigned short id /*= 0*/)
{
    if (id != 0)
	{
		m_id = id;
	}
	else if(m_id == 0)
    {
        m_id = rand() % (65535 + 1);
    }
    
    return m_id;
}

// Note: the 16bits flag is not short int value, no endianess issue
int dns::Header::toBuffer(unsigned char* buf, size_t size)
{
    if (size >= HEADER_LENGTH)
    {
        uint16_t flag = flag_enc();
        
        uint16_t* p = (uint16_t*)buf;
        *(p++) = htons(m_id);
        *(p++) = flag;
        *(p++) = htons(m_qdcount); // question count
        *(p++) = htons(m_ancount); // answer count
        *(p++) = htons(m_nscount);
        *(p++) = htons(m_arcount);
        return HEADER_LENGTH;
    }
    return -1;
}

// Note: the 16bits flag is not short int value, no endianess issue
bool dns::Header::fromBuffer(unsigned char* buf, size_t size, size_t& offset)
{
    if((size - offset) < HEADER_LENGTH)
    {
        std::cout << "Buffer is too small to decode header" << std::endl;
        return false;
    }

    uint16_t* p = (uint16_t*)(buf + offset);
    m_id = ntohs(*(p++));
    uint16_t flag = *(p++);
    m_qdcount = ntohs(*(p++));
    m_ancount = ntohs(*(p++));
    m_nscount = ntohs(*(p++));
    m_arcount = ntohs(*(p++));
    offset += HEADER_LENGTH;
    
    flag_dec(flag);
    
    return true;
}

std::string dns::Header::toString()
{
    std::ostringstream oss;
    if(m_flags.qr)
    {
        oss << "// Header Response ";
        switch (m_flags.rcode) 
        {
            case DNS_RESPONSE_NO_ERROR:
                oss << "PASS";
                break;
            case DNS_RESPONSE_FORMAT_ERROR:
                oss << "FORMAT ERROR ";
                break;
            case DNS_RESPONSE_SERVER_FAILURE:
                oss << "SERVER FAILURE ";
                break;
            case DNS_RESPONSE_NAME_ERROR:
                oss << "NAME ERROR ";
                break;
            case DNS_RESPONSE_NOT_IMPLEMENTED:
                oss << "NOT IMPLEMENTED ";
                break;
            case DNS_RESPONSE_REFUSED:
                oss << "REFUSED ";
                break;    
                
            default:
                break;
        }
        
        oss << std::endl;
    }
    else
    {
        oss << "// Request " << std::endl;;
    }
    
    oss << m_id << " "
        << m_qdcount << " "
        << m_ancount << " "
        << m_nscount << " "
        << m_arcount;
        
    return oss.str();
}

/*
                                    1  1  1  1  1  1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*/
void dns::Header::flag_dec(uint16_t flag)
{
	uint8_t* p = (uint8_t*)&flag;
    m_flags.qr = p[0] & 0x80;
    m_flags.opcode = (p[0] & 0x78) >> 3;
    m_flags.aa = p[0] & 0x04;
    m_flags.tc = p[0] & 0x02;
    m_flags.rd = p[0] & 0x01;
    
    m_flags.ra = p[1] & 0x80;
    m_flags.z = (p[1] & 0x70) >> 4;
    m_flags.rcode = p[1] & 0x0f;
}

// Encode flags into 2-byte value
uint16_t dns::Header::flag_enc()
{
	uint16_t flag;
	uint8_t* p = (uint8_t*)&flag;
    p[0] = (m_flags.rd
          	| (m_flags.tc << 1)
            | (m_flags.aa << 2)
            | (m_flags.opcode << 3)
            | (m_flags.qr << 7));
    
    p[1] = (m_flags.rcode
            | (m_flags.z << 4)
            | (m_flags.ra << 7));
	return flag;
}



