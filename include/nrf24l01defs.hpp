/*
    Copyright (c) 2007 Stefan Engelke <mbox@stefanengelke.de>

    Permission is hereby granted, free of charge, to any person 
    obtaining a copy of this software and associated documentation 
    files (the "Software"), to deal in the Software without 
    restriction, including without limitation the rights to use, copy, 
    modify, merge, publish, distribute, sublicense, and/or sell copies 
    of the Software, and to permit persons to whom the Software is 
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be 
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
    DEALINGS IN THE SOFTWARE.

    $Id$
*/

#ifndef NRF24L01DEFS_HPP
#define NRF24L01DEFS_HPP

namespace nrf24l01 {
static const int config = 0x00;
static const int en_aa = 0x01;
static const int en_rxaddr = 0x02;
static const int setup_aw = 0x03;
static const int setup_retr = 0x04;
static const int rf_ch = 0x05;
static const int rf_setup = 0x06;
static const int status = 0x07;
static const int observe_tx = 0x08;
static const int cd = 0x09;
static const int rx_addr_p0 = 0x0a;
static const int rx_addr_p1 = 0x0b;
static const int rx_addr_p2 = 0x0c;
static const int rx_addr_p3 = 0x0d;
static const int rx_addr_p4 = 0x0e;
static const int rx_addr_p5 = 0x0f;
static const int tx_addr = 0x10;
static const int rx_pw_p0 = 0x11;
static const int rx_pw_p1 = 0x12;
static const int rx_pw_p2 = 0x13;
static const int rx_pw_p3 = 0x14;
static const int rx_pw_p4 = 0x15;
static const int rx_pw_p5 = 0x16;
static const int fifo_status = 0x17;
static const int mask_rx_dr = 6;
static const int mask_tx_ds = 5;
static const int mask_max_rt = 4;
static const int en_crc = 3;
static const int crco = 2;
static const int pwr_up = 1;
static const int prim_rx = 0;
static const int enaa_p5 = 5;
static const int enaa_p4 = 4;
static const int enaa_p3 = 3;
static const int enaa_p2 = 2;
static const int enaa_p1 = 1;
static const int enaa_p0 = 0;
static const int erx_p5 = 5;
static const int erx_p4 = 4;
static const int erx_p3 = 3;
static const int erx_p2 = 2;
static const int erx_p1 = 1;
static const int erx_p0 = 0;
static const int aw = 0;
static const int ard = 4;
static const int arc = 0;
static const int pll_lock = 4;
static const int rf_dr_low = 5;
static const int rf_dr_high = 3;
static const int rf_pwr = 1;
static const int lna_hcurr = 0; 
static const int rx_dr = 6;
static const int tx_ds = 5;
static const int max_rt = 4;
static const int rx_p_no = 1;
static const int tx_full = 0;
static const int plos_cnt = 4;
static const int arc_cnt = 0;
static const int tx_reuse = 6;
static const int fifo_full = 5;
static const int tx_empty = 4;
static const int rx_full = 1;
static const int rx_empty = 0;
static const int r_register = 0x00;
static const int w_register = 0x20;
static const int register_mask = 0x1f;
static const int r_rx_payload = 0x61;
static const int w_tx_payload = 0xa0;
static const int flush_tx = 0xe1;
static const int flush_rx = 0xe2;
static const int reuse_tx_pl = 0xe3;
static const int nop = 0xff;
};

#endif
