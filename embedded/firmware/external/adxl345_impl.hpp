// coding: utf-8
// ----------------------------------------------------------------------------
/* Copyright (c) 2011, Roboterclub Aachen e.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Roboterclub Aachen e.V. nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ROBOTERCLUB AACHEN E.V. ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ROBOTERCLUB AACHEN E.V. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

 /*---------------------------------+
  | This file was modified by       |
  | Supreme Machines                |
  | Matthias Kubisch                |
  | kubisch@informatik.hu-berlin.de |
  | October 2018                    |
  +---------------------------------*/


#ifndef SUPREME_ADXL345_HPP
#	error  "Don't include this file directly, use 'adxl345.hpp' instead!"
#endif

// ----------------------------------------------------------------------------
template < typename I2cMaster >
xpcc::Adxl345<I2cMaster>::Adxl345(uint8_t* data, uint8_t address)
: xpcc::I2cDevice<I2cMaster, 2>(address), status(0), data(data)
{
	this->transaction.configureWriteRead(buffer, 0, data, 0);
}

/* blocking */
template < typename I2cMaster >
bool
xpcc::Adxl345<I2cMaster>::configure(adxl345::Bandwidth bandwidth, bool streamMode, bool enableInterrupt)
{
	bool ok = writeRegister(adxl345::REGISTER_POWER_CTL, adxl345::POWER_MEASURE);
	ok &= writeRegister(adxl345::REGISTER_DATA_FORMAT, adxl345::DATAFORMAT_FULL_RES);
	ok &= writeRegister(adxl345::REGISTER_BW_RATE, bandwidth);
	if (enableInterrupt) ok &= writeRegister(adxl345::REGISTER_INT_ENABLE, adxl345::INTERRUPT_DATA_READY);
	if (streamMode) ok &= writeRegister(adxl345::REGISTER_FIFO_CTL, adxl345::FIFO_CTL_MODE_STREAM);
	return ok;
}

template < typename I2cMaster >
void
xpcc::Adxl345<I2cMaster>::readAccelerometer()
{
	status |= READ_ACCELEROMETER_PENDING;
}

template < typename I2cMaster >
bool
xpcc::Adxl345<I2cMaster>::isDataReady()
{
	return readRegister(adxl345::REGISTER_INT_SOURCE) & adxl345::INTERRUPT_DATA_READY;
}

template < typename I2cMaster >
bool
xpcc::Adxl345<I2cMaster>::isNewDataAvailable()
{
	return status & NEW_ACCELEROMETER_DATA;
}

template < typename I2cMaster >
uint8_t*
xpcc::Adxl345<I2cMaster>::getData()
{
	status &= ~NEW_ACCELEROMETER_DATA;
	return data;
}

/* non-blocking */
template < typename I2cMaster >
xpcc::ResumableResult<void>
xpcc::Adxl345<I2cMaster>::update()
{
	RF_BEGIN();
	if (status & READ_ACCELEROMETER_RUNNING &&
		this->transaction.getState() == xpcc::I2c::TransactionState::Idle) {
		status &= ~READ_ACCELEROMETER_RUNNING;
		status |= NEW_ACCELEROMETER_DATA;
	}
	else if (status & READ_ACCELEROMETER_PENDING)
	{
		buffer[0] = adxl345::REGISTER_DATA_X0;
		this->transaction.configureWriteRead(buffer, 1, data, 6);

		if (RF_CALL( this->runTransaction() )) {
			status &= ~READ_ACCELEROMETER_PENDING;
			status |= READ_ACCELEROMETER_RUNNING;
		}
	}
	RF_END();
}

/* blocking */
template <typename I2cMaster>
bool
xpcc::Adxl345<I2cMaster>::writeRegister(adxl345::Register reg, uint8_t value)
{
	while (this->transaction.getState() == xpcc::I2c::TransactionState::Busy)
		;
	buffer[0] = reg;
	buffer[1] = value;
	this->transaction.configureWriteRead(buffer, 2, data, 0);

	return RF_CALL_BLOCKING( this->runTransaction() );
}

/* blocking */
template <typename I2cMaster>
uint8_t
xpcc::Adxl345<I2cMaster>::readRegister(adxl345::Register reg)
{
	while (this->transaction.getState() == xpcc::I2c::TransactionState::Busy)
		;
	buffer[0] = reg;
	this->transaction.configureWriteRead(buffer, 1, buffer, 1);

	RF_CALL_BLOCKING( this->runTransaction() );
	return buffer[0];
}
