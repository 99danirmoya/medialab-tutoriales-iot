/* ***********************************************************************************************************************************************************
TTN module
Wrapper to use TTN with the LMIC library

Copyright (C) 2019 by Roel van Wanrooy (www.connectix.nl)

This code requires the MCCI LoRaWAN LMIC library by IBM, Matthis Kooijman, Terry Moore, ChaeHee Won, Frank Rose https://github.com/mcci-catena/arduino-lmic

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************************************************************************************************** */
#pragma once

void ttn_register(void (*callback)(uint8_t message));
void ttn_cnt(unsigned char num);
void ttn_send(uint8_t * data, uint8_t data_size, uint8_t port, bool confirmed);
void ttn_response(uint8_t * buffer, size_t len);
size_t ttn_response_len();