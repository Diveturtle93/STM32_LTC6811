//----------------------------------------------------------------------
// Titel	:	isospi.h
//----------------------------------------------------------------------
// Sprache	:	C
// Datum	:	21.06.2026
// Autor	:	Diveturtle93
// Projekt	:	LTC6811
// Quelle	:
//----------------------------------------------------------------------

// Sicherheitssymbol
//----------------------------------------------------------------------
#pragma once
//----------------------------------------------------------------------

// Dateiheader definieren
//----------------------------------------------------------------------
#ifndef INC_ISOSPI_H_
#define INC_ISOSPI_H_
//----------------------------------------------------------------------

// Einfuegen der standard Include-Dateien
//----------------------------------------------------------------------

//----------------------------------------------------------------------

// Einfuegen der STM Include-Dateien
//----------------------------------------------------------------------

//----------------------------------------------------------------------

// Einfuegen der eigenen Include-Dateien
//----------------------------------------------------------------------

//----------------------------------------------------------------------

// Definiere Debug-Symbole
//----------------------------------------------------------------------
#ifdef DEBUG
//	#define DEBUG_ISOSPI
#endif
//----------------------------------------------------------------------

// Definiere ISO-SPI Enable Leitung, LTC6811
//----------------------------------------------------------------------
#define ISOSPI_ENABLE()							(HAL_GPIO_WritePin(ISOSPI_EN_GPIO_Port, ISOSPI_EN_Pin, GPIO_PIN_SET))		// Iso SPI enable
#define ISOSPI_DISABLE()						(HAL_GPIO_WritePin(ISOSPI_EN_GPIO_Port, ISOSPI_EN_Pin, GPIO_PIN_RESET))	// Iso SPI disable
//----------------------------------------------------------------------

// Definiere Chip-Select Leitung, LTC6811
//----------------------------------------------------------------------
#define ISOCS_ENABLE()							(HAL_GPIO_WritePin(ISOSPI_CS_GPIO_Port, ISOSPI_CS_Pin, GPIO_PIN_RESET))		// Chip-Select enable
#define ISOCS_DISABLE()							(HAL_GPIO_WritePin(ISOSPI_CS_GPIO_Port, ISOSPI_CS_Pin, GPIO_PIN_SET))		// Chip-Select disable
//----------------------------------------------------------------------

// Definiere IsoSpi Statemaschine Typedefines
//----------------------------------------------------------------------
typedef enum IsoSpiState_tag
{
	IsoIdle,									// 1						// Kommunikation unterbrochen
	IsoReady,									// 2						// Kommunikation kann durchgefuehrt werden
	IsoActive,									// 3						// Kommunikation wird durchgefuehrt
	IsoGetReady,								// 4						// Kommunikation wird vorbereitet
	IsoSleep,									// 5						// Kommunikation abgeschaltet
} IsoSpi_State;
//----------------------------------------------------------------------

// Funktionen definieren
//----------------------------------------------------------------------
void set_IsoSpiState (IsoSpi_State newState);								// Setze neuen State von IsoSPI
void IsoSPI_statemaschine (void);											// Auswertung State von IsoSPI
IsoSpi_State get_IsoSpiState (void);										// Bekomme aktuellen State von IsoSPI
void IsoSPI_wakeup (void);													// Aufwachfunktion fuer IsoSPI
void IsoSPI_cmd (uint8_t* command);											// Sende Command ueber IsoSPI
void IsoSPI_transmit (uint8_t* command, uint8_t* data);						// Sende Daten ueber IsoSPI
void IsoSPI_read (uint8_t* command, uint8_t* data);							// Lese Daten ueber IsoSPI
//----------------------------------------------------------------------

#endif /* INC_ISOSPI_H_ */
//----------------------------------------------------------------------
