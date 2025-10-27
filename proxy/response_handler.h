/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/file.h to edit this template
 */

/* 
 * File:   response_handler.h
 * Author: hamza
 *
 * Created on October 22, 2025, 11:24 AM
 */

#ifndef RESPONSE_HANDLER_H
#define RESPONSE_HANDLER_H

#include <pjsua2.hpp>
using namespace std;

class ResponseHandler {
public:

    ResponseHandler() {
    };

    ~ResponseHandler() {
    };

    void generate_quick_response(pjsip_rx_data *rdata, string proxyIp);
    void forward_request(pjsip_rx_data *rdata, string proxyIp);
    void forward_response(pjsip_rx_data *rdata, string proxyIp, pjsip_transaction *uas_tsx);


};

#endif /* RESPONSE_HANDLER_H */

