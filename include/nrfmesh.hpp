#ifndef NRFMESH_HPP
#define NRFMESH_HPP
#include "nrf24l01.hpp"
#include "rand.hpp"

namespace nrf24l01 {
/**
 *  Class that implements a mesh network on top of nrf24l01 radio
 */
template<
    uint8_t channel_ = 10,
    uint8_t payload_size_ = 4,
    uint8_t packet_dedup_buf_ = 6,  // 1 -> 2, 2 -> 4, 3 -> 8 etc
    uint8_t address_length_ = 5,
    uint8_t csn_pin_ = 7,
    uint8_t ce_pin_ = 8
>
class nrfmesh {
    static constexpr uint8_t buf_size = 1<<packet_dedup_buf_;
    struct __attribute__((packed)) packet {
        uint8_t sequence_no;            // Used to avoid two equal packets being flagged as duplicate
        uint8_t data[payload_size_];
        };

    nrf24l01<
        channel_,
        payload_size_+1,
        address_length_,
        2,                  // CRC length, ensures low corruption
        false,              // We use broadcast, so no autoack needed (or possible)
        csn_pin_,
        ce_pin_> nrf;

    packet buf[buf_size];
    uint8_t buf_pos;
    packet to_send;
    rng rand;

    void transmit(uint8_t* pkt) {
        for (uint8_t i=0; i<4; i++) {
            for (uint8_t _=0; _<4; i++) {
                uint16_t wait_time = rand();
                delayMicroseconds(wait_time<<(4-i));
                nrf.tx_mode();
                nrf.send_sync(pkt);
                nrf.rx_mode();
            }
        }
    }

public:
    static constexpr uint8_t payload_size = payload_size_; 
    /**
     * Constructor.
     *
     * @param addr the address of the mesh
     * @param seed seed for the rng that generates the delays
     */
    nrfmesh(const uint8_t* addr, uint8_t seed): buf_pos(0), rand(seed) {
        nrf.set_tx_addr((uint8_t*)addr);
        nrf.rx_mode();
        to_send.sequence_no = rand();
    }

    uint8_t* get_next_packet() {
        return to_send.data;
    }

    void send() {
        memcpy(&buf[buf_pos], &to_send, sizeof(packet));
        buf_pos = (buf_pos+1) & (buf_size-1);
        transmit((uint8_t*)&to_send);
        to_send.sequence_no++;
    }

    /**
     * Tries to receive a packet.
     *
     * Returns null if there is no available packet or if
     * we received a duplicate packet. Otherwise returns a
     * pointer that is valid until (at least) the next call
     * to try_receive or send.
     */
    uint8_t* try_receive() {
        if (nrf.packet_available() == -1) return nullptr;
        nrf.receive((uint8_t*)&buf[buf_pos]);
        bool found = false;
        for (int i=0; i<buf_size; i++) {
            if (i != buf_pos && memcmp((uint8_t*)&buf[buf_pos], (uint8_t*)&buf[i], sizeof(packet)) == 0) {
                found = true;
                break;
            }
        }
        if (found) {
            return nullptr;
        }
        transmit((uint8_t*)&buf[buf_pos]);
        uint8_t* ret = buf[buf_pos].data;
        buf_pos = (buf_pos+1) & (buf_size-1);
        return ret;
    }
};
}

#endif
