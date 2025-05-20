# 🕵️‍♂️ ESP32 Timing Side-Channel Attack — Flow Documentation

This document explains the internal logic, flow, and design of a **precise timing side-channel attack** using an ESP32 attacker and ESP8266 victim. The attack exploits **timing variations** in password validation to infer the secret character-by-character.

---

## 📜 Overview

- **Attacker:** ESP32
- **Victim:** ESP8266
- **Goal:** Extract a secret password by measuring response times for different guesses.
- **Attack Type:** Timing-based side-channel attack over Wi-Fi (local network)

---

## ⚙️ High-Level Architecture


---

## 🔄 Attack Flow

### 1. Initialization
- ESP32 connects to a local Wi-Fi network.
- Target IP and port of ESP8266 are set.

### 2. Iterative Guessing
- The attack starts at position 0 of the password.
- For each character in the charset (e.g., `a-z`, `A-Z`, `0-9`, symbols):
  - Guess = current known prefix + test character.
  - Send the guess to the ESP8266.
  - Measure how long it takes to respond.
  - Repeat for `N` samples (e.g., 30) to build reliable timing stats.

### 3. Timing Analysis
- Calculate:
  - **Average**
  - **Minimum**
  - **Maximum**
  - **Standard deviation**
- Choose the character with the **highest average time** as most likely correct.
- If timing difference vs. next best is **significant (e.g., >30%)**, confirm it.

### 4. Confirm and Proceed
- Append confirmed character to password guess.
- Move to the next index.
- Repeat the process until:
  - A clear timing difference is no longer visible.
  - Or a preset max length is reached.

### 5. Completion
- Output final guessed password (e.g., `secret42`).
- Optionally log all timings for further visualization.

---

## 🧪 Sample Output

