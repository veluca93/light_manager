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
    uint8_t packet_dedup_buf_send_ = 4,  // 1 -> 2, 2 -> 4, 3 -> 8 etc
    uint8_t address_length_ = 5,
    uint8_t csn_pin_ = 7,
    uint8_t ce_pin_ = 8
>
class nrfmesh {
    static constexpr uint8_t buf_size = 1<<packet_dedup_buf_;
    static constexpr uint8_t buf_size_send = 1<<packet_dedup_buf_send_;
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
    uint8_t buf_end;
    uint8_t buf_first;
    packet buf_send[buf_size_send];
    uint8_t buf_pos_send;
    rng rand;

    void transmit(uint8_t* pkt) {
        for (int i=0; i<16; i++) {
            uint16_t wait_time = rand();
            delayMicroseconds(wait_time>>5);
            nrf.tx_mode();
            nrf.send_sync(pkt);
            nrf.rx_mode();
            flush();
        }
    }

    bool equals(packet& a, packet& b) {
        return memcmp((uint8_t*)&a, (uint8_t*)&b, sizeof(packet)) == 0;
    }

    /**
     * Moves a single packet from the nrf24l01 queue to ours.
     *
     * @return true if a packet was enqueued successfully
     */
    bool enqueue() {
        if (nrf.packet_available() == -1) return false;
        nrf.receive((uint8_t*)&buf[buf_end]);
        for (int i=0; i<buf_size; i++) {
            if (i != buf_end && equals(buf[buf_end], buf[i])) {
                return false;
            }
        }
        for (int i=0; i<buf_size_send; i++) {
            if (equals(buf[buf_end], buf_send[i])) {
                return false;
            }
        }
        buf_end = (buf_end+1) & (buf_size-1);
        return true;
    }

public:
    static constexpr uint8_t payload_size = payload_size_; 
    /**
     * Constructor.
     *
     * @param addr the address of the mesh
     * @param seed seed for the rng that generates the delays
     */
    nrfmesh(const uint8_t* addr, uint8_t seed)
    : buf_end(0), buf_first(0), buf_pos_send(0), rand(seed) {
        nrf.set_tx_addr((uint8_t*)addr);
        nrf.rx_mode();
        buf_send[buf_pos_send].sequence_no = rand();
    }

    uint8_t* get_next_packet() {
        return buf_send[buf_pos_send].data;
    }

    void send() {
        uint8_t next_buf_pos = (buf_pos_send+1) & (buf_size_send-1);
        transmit((uint8_t*)&buf_send[buf_pos_send]);
        memcpy(&buf_send[next_buf_pos], &buf_send[buf_pos_send], sizeof(packet));
        buf_send[next_buf_pos].sequence_no++;
        buf_pos_send = next_buf_pos;
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
        if (buf_first == buf_end) {
            if (enqueue() == false) {
                return nullptr;
            }
        }
        // Removes a packet from the queue,
        // sends it to neighbours and returns it
        // to the caller
        transmit((uint8_t*)&buf[buf_first]);
        uint8_t* ret = buf[buf_first].data;
        buf_first = (buf_first+1) & (buf_size-1);
        return ret;
    }

    void flush() {
        // Clear the radio's queue
        while(enqueue());
    }
};
}

#endif
