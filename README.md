# 🔓 ESP32 Precise Timing Side-Channel Attack

This repository demonstrates a practical **timing side-channel attack** using an **ESP32 as the attacker** and an **ESP8266 as the vulnerable target**. The attacker deduces a secret password character-by-character by analyzing response time variations over Wi-Fi.

---

## 🚀 Project Structure

![architecture](https://github.com/user-attachments/assets/fda3f676-b7de-4543-b830-a53a30dfa52f)

---

---

## 🎯 Objective

> **Extract a secret password (e.g., `secret42`) from the ESP8266**  
> using only timing variations in its response to login attempts.

---

## 🛠️ Requirements

### Hardware
- 🔹 ESP32 (e.g., DevKit V1)
- 🔹 ESP8266 (e.g., NodeMCU)
- 🔹 Shared Wi-Fi connection

### Software
- Arduino IDE with:
  - ESP32 board package
  - ESP8266 board package
- Python 3.7+
  - `matplotlib`
  - `pandas`

Install dependencies:

```bash
pip install matplotlib pandas
```
---
## 🔄 Attack Flow
1. ESP32 connects to Wi-Fi and targets the ESP8266 server.
2. It sends login attempts with different guesses at each character position.
3. Measures the time taken for each response using micros().
4. Builds a statistical profile of response times per character.
5. Chooses the character with the highest average time as the likely correct one.
6. Repeats for each subsequent position until the full password is revealed.

## 🛡️ Defenses & Mitigations
- Use constant-time string comparisons
- Introduce artificial random delays
- Perform rate-limiting and lockouts on repeated guesses
- Avoid using strcmp() or similar early-return logic in auth routines
---
© 2025 intelligent-ears. All rights reserved.
