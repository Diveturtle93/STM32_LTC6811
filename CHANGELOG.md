# Changelog

Alle nennenswerten Änderungen an diesem Projekt werden hier dokumentiert.
Format nach [Keep a Changelog](https://keepachangelog.com/de/1.1.0/),
Versionierung nach [Semantic Versioning](https://semver.org/lang/de/).

## [Unveröffentlicht]

### Hinzugefügt
- Ansteuerung eines oder mehrerer LTC6811 im Daisy-Chain über isoSPI (`isospi.c` / `isospi.h`) mit eigener Statemaschine für Wecken, Bereitschaft und Übertragung.
- Statemaschine für den LTC6811 mit den Zuständen Standby, Refup, Messung, Extended Balancing und Sleep inklusive Watchdog-Timeout.
- Lese-, Schreib- und Kommandofunktionen für alle Registergruppen des LTC6811 (Konfiguration, Zellspannungen, Auxiliary, Status, S-Control, PWM, Communication).
- PEC-Berechnung (CRC-15) über eine Lookup-Tabelle für Kommandos und Daten sowie PEC-Prüfung empfangener Daten je LTC6811.
- Initialisierung des Konfigurationsregisters mit einstellbaren Grenzwerten für Unter- und Überspannung (`LTC6811_UVOLT`, `LTC6811_OVOLT`).
- Diagnosefunktionen: ADC-Selbsttests, Thermal Shutdown, Multiplexer-Diagnose und Open-Wire-Erkennung, zusammengefasst in `ltc6811_check()`.
- Balancing über `ltc6811_balancing()` und Prüfung der Balancing-MOSFETs über `ltc6811_validate_balance()`.
- Abfrage des ADC-Wandlungsstatus über `ltc6811_poll()`.
- Ansteuerung von LTC1380-Multiplexern über den I2C-Master des LTC6811 (`ltc1380.c` / `ltc1380.h`) zur Auswahl von Temperatursensoren.
- Debug-Ausgaben über `DEBUG_ISOSPI`, `DEBUG_LTC6811`, `DEBUG_LTC6811_PEC`, `DEBUG_LTC6811_VALID_BALANCING` und `DEBUG_LTC1380`.
- Versions-Defines `LTC6811_MAJOR`, `_MINOR`, `_PATCH` und `_DEV` in `ltc6811.h`.
