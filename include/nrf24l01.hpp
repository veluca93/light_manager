#ifndef NRF24L01_HPP
#define NRF24L01_HPP

#include <SPI.h>
#include <Arduino.h>
#include "nrf24l01defs.hpp"

/**
 *  Class for communication with the nrf24l01 radio, inspired by Mirf.
 */
namespace nrf24l01 {

template <
    uint8_t channel_ = 10,
    uint8_t payload_size_ = 16,
    uint8_t address_length_ = 5, // From 3 to 5
    uint8_t crc_length_ = 2, // Legal values from 0 to 2
    bool enable_autoack_ = true,
    uint8_t csn_pin_ = 7,
    uint8_t ce_pin_ = 8,
    uint8_t retransmit_delay_ = 3, // From 0 to 15, increased by one and multiplied by 250uS
    uint8_t retransmit_count_ = 3  // From 0 to 15
>
class nrf24l01 {
    constexpr static uint8_t gen_base_config() {
        static_assert(crc_length < 3, "Invalid crc length");
        static_assert(!enable_autoack || crc_length, "If autoack is enabled,"
            " CRC must be enabled too.");
        if (crc_length == 0) return 0;
        uint8_t ans = 1 << en_crc;
        if (crc_length == 2) ans |= 1 << crco;
        return ans;
    }


    static constexpr uint8_t channel = channel_;
    static constexpr uint8_t csn_pin = csn_pin_;
    static constexpr uint8_t ce_pin = ce_pin_;
    static constexpr uint8_t crc_length = crc_length_;
    static constexpr uint8_t enable_autoack = enable_autoack_;
    static constexpr uint8_t status_clear = (1<<tx_ds) | (1<<max_rt);
    static constexpr uint8_t rxdr_clear = (1<<rx_dr);
    static constexpr uint8_t off_config = gen_base_config();
    static constexpr uint8_t tx_config = off_config | (1<<pwr_up);
    static constexpr uint8_t rx_config = tx_config | (1<<prim_rx);
    static constexpr uint8_t retr_config = (retransmit_delay_<<4) | retransmit_count_;

    uint8_t enabled_pipes = 0;

    template <uint8_t pin, bool high>
    class PinGuard {
    public:
        PinGuard() {
            digitalWrite(pin, high ? HIGH : LOW);
        }
        ~PinGuard() {
            digitalWrite(pin, high ? LOW : HIGH);
        }
    };

    template<uint8_t len>
    void transfer(const uint8_t* to_send) {
        for (uint8_t i=0; i<len; i++) {
            SPI.transfer(to_send[i]);
        }
    }

    template<uint8_t len>
    void transfer(uint8_t to_send) {
        static_assert(len == 1, "You can use this function only for len==1.");
        SPI.transfer(to_send);
    }

    template<uint8_t len>
    void transfer(const uint8_t* to_send, uint8_t* received) {
        for (uint8_t i=0; i<len; i++) {
            received[i] = SPI.transfer(to_send[i]);
        }
    }

    template<uint8_t len, typename data_t>
    void write_register(uint8_t reg, data_t data) {
        PinGuard<csn_pin, false> pg;
        SPI.transfer(w_register | (register_mask & reg));
        transfer<len>(data);
    }
    
    template<uint8_t len>
    void read_register(uint8_t reg, uint8_t* data) {
        PinGuard<csn_pin, false> pg;
        SPI.transfer(r_register | (register_mask & reg));
        transfer<len>(data, data);
    }

    void enable_pipe(int pipe_n) {
        if (enabled_pipes & (1<<pipe_n)) return;
        enabled_pipes |= 1<<pipe_n;
        write_register<1>(en_rxaddr, &enabled_pipes);
    }

    uint8_t get_status() {
        uint8_t rv = 0;
        read_register<1>(status, &rv);
        return rv;
    }

    bool rx_fifo_empty() {
        uint8_t rv = 0;
        read_register<1>(fifo_status, &rv);
        return (rv & (1<<rx_empty));
    }
public:
    static constexpr uint8_t payload_size = payload_size_;
    static constexpr uint8_t address_length = address_length_;
    static constexpr uint8_t retransmit_delay = retransmit_delay_;
    static constexpr uint8_t retransmit_count = retransmit_count_;

    nrf24l01() {
        static_assert(3 <= address_length && address_length <= 5,
            "Address length must be between 3 and 5");
        static_assert(retransmit_count < 16,
            "Retransmit count must be between 0 and 15");
        static_assert(retransmit_delay < 16,
            "Retransmit delay must be between 0 and 15");
        pinMode(ce_pin, OUTPUT);
        pinMode(csn_pin, OUTPUT);
        digitalWrite(ce_pin, LOW);
        digitalWrite(csn_pin, HIGH);
        SPI.begin();
        SPI.setDataMode(SPI_MODE0);
        SPI.setClockDivider(SPI_2XCLOCK_MASK);
        off_mode();
        write_register<1>(status, status_clear);
        // Set channel
        write_register<1>(rf_ch, channel);
        // Set transmission speed and power
        write_register<1>(rf_setup, (3<<rf_pwr) | (1<<rf_dr_low));
        // Set payload length
        write_register<1>(rx_pw_p0, payload_size);
        write_register<1>(rx_pw_p1, payload_size);
        write_register<1>(rx_pw_p2, payload_size);
        write_register<1>(rx_pw_p3, payload_size);
        write_register<1>(rx_pw_p4, payload_size);
        write_register<1>(rx_pw_p5, payload_size);
        // Possibly enable autoack
        if (enable_autoack) {
            const uint8_t autoack_data = 0x3F;
            write_register<1>(en_aa, autoack_data);
        } else {
            write_register<1>(en_aa, 0);
        }
        // Disable data pipes
        write_register<1>(en_rxaddr, enabled_pipes);
        // Set up address length
        const uint8_t addr_len = address_length - 2;
        write_register<1>(setup_aw, addr_len);
        SPI.transfer(flush_rx);
        write_register<1>(setup_retr, 0xFF);
    }

    /**
     *  We set P0 equal to the transmit address. All other
     *  receive addresses can be set indipendently.
     */
    void set_tx_addr(uint8_t* addr) {
        write_register<address_length>(rx_addr_p0, addr);
        write_register<address_length>(tx_addr, addr);
        enable_pipe(0);
    }

    void set_rx_addr(uint8_t pipe, uint8_t* addr) {
        // TODO: check pipe < 5 in debug mode
        write_register<address_length>(rx_addr_p1+pipe, addr);
        read_register<address_length>(rx_addr_p1+pipe, addr);
        // Enable the data pipe
        enable_pipe(pipe+1);
    }

    /**
     *  Mode control functions.
     */
    void rx_mode() {
        digitalWrite(ce_pin, LOW);
        write_register<1>(status, rxdr_clear);
        write_register<1>(config, rx_config);
        digitalWrite(ce_pin, HIGH);
        //SPI.transfer(flush_rx);
    }

    void tx_mode() {
        digitalWrite(ce_pin, LOW);
        write_register<1>(status, status_clear);
        write_register<1>(config, tx_config);
        SPI.transfer(flush_tx);
    }

    void off_mode() {
        digitalWrite(ce_pin, LOW);
        write_register<1>(config, off_config);
    }

    /**
     *  Check if a packet is available. Returns -1 if not, the number of the data pipe otherwise.
     */
    int packet_available() {
        uint8_t status = get_status();
        //Serial.println(status, BIN);
        status &= 0b00001110;
        status >>= 1;
        return status == 7 ? -1 : status;
    }

    /**
     *  Gets a data packet
     */
    void receive(uint8_t* data) {
        //TODO: check there is a packet available in debug mode
        {
            PinGuard<csn_pin, false> pg;
            SPI.transfer(r_rx_payload);
            transfer<payload_size>(data, data);
        }
    }

    /**
     * Returns 0 if the packet was sent successfully, 1 if still sending,
     * -1 if there was an error.
     */
    int send_status() {
        uint8_t status = get_status();
        //Serial.println(status, BIN);
        if (status & (1<<tx_ds)) {
            off_mode();
            tx_mode();
            return 0;
        }
        if (status & (1<<max_rt)) {
            off_mode();
            tx_mode();
            return -1;
        }
        return 1;
    }

    /**
     *  Sends a packet asynchronously. Be sure to be in transmit mode
     *  and that you have no currently unsent packets.
     */
    void send(const uint8_t* data) {
        {
            PinGuard<csn_pin, false> pg;
            SPI.transfer(flush_tx);
        }
        {
            PinGuard<csn_pin, false> pg;
            SPI.transfer(w_tx_payload);
            transfer<payload_size>(data);
        }
        PinGuard<ce_pin, true> cepg;
        delayMicroseconds(15);
    }

    /**
     *  Sends a packet and waits until it is done. Returns true if the packet
     *  was sent successfully, false otherwise.
     */
    bool send_sync(const uint8_t* data) {
        int status = 0;
        get_status();
        send(data);
        while ((status = send_status()) == 1);
        return !status;
    }

    /**
     *  Receives a packet waiting for one to be available.
     *  
     *  Returns the data pipe on which the packet was received.
     */
    uint8_t receive_sync(uint8_t* data) {
        int pipe_n = -1;
        while ((pipe_n = packet_available()) == -1);
        receive(data);
        return pipe_n;
    }

    class sender {
        friend class nrf24l01;
        nrf24l01& n;
        sender(nrf24l01& n): n(n) {
            n.tx_mode();
        }
        ~sender() {
            n.rx_mode();
        }
        sender(const sender&) = delete;
    public:
        bool send_sync(const uint8_t* data) {
            return n.send_sync(data);
        }
    };

    sender get_sender() {
        return sender(*this);
    }
};


};

#endif
