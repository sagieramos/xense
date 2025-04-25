# 📡 Wi-Fi Cipher Types & Auth Modes

This document explains the **Wi-Fi cipher types** and **authentication modes** used in the ESP32 firmware. These help determine encryption, security level, and whether a password is required.

---

## 🔐 Cipher Types

These define how the connection is encrypted.

| Value | Constant | Description | Password Required |
|-------|----------|-------------|-------------------|
| 0 | `NONE` | No encryption (open network) | ❌ No |
| 1 | `WEP40` | WEP-40 encryption (insecure) | ✅ Yes |
| 2 | `WEP104` | WEP-104 encryption (insecure) | ✅ Yes |
| 3 | `TKIP` | TKIP encryption (used in WPA) | ✅ Yes |
| 4 | `CCMP` | AES encryption (used in WPA2) | ✅ Yes |
| 5 | `TKIP_CCMP` | Supports both TKIP and CCMP | ✅ Yes |
| 6 | `AES_CMAC128` | Enterprise-level encryption | ✅ Yes |
| 7 | `SMS4` | WAPI encryption (China standard) | ✅ Yes |
| 8 | `GCMP` | High-performance encryption (WPA3) | ✅ Yes |
| 9 | `GCMP256` | Stronger version of GCMP | ✅ Yes |
| 10 | `AES_GMAC128` | Used for integrity/auth | ✅ Yes |
| 11 | `AES_GMAC256` | Stronger version of GMAC128 | ✅ Yes |
| 12 | `NKNOWN` | Unknown or unsupported | ❓ Unknown |

---

## 🔒 Authentication Modes (`wifi_auth_mode_t`)

These define how devices authenticate with the Wi-Fi router.

| Value | Constant | Description | Password Required |
|-------|----------|-------------|-------------------|
| 0 | `OPEN` | Open network | ❌ No |
| 1 | `WEP` | WEP security (insecure) | ✅ Yes |
| 2 | `WPA_PSK` | WPA with Pre-Shared Key | ✅ Yes |
| 3 | `WPA2_PSK` | WPA2 with Pre-Shared Key | ✅ Yes |
| 4 | `WPA_WPA2_PSK` | WPA or WPA2 (auto-negotiated) | ✅ Yes |
| 5 | `ENTERPRISE` | WPA2 Enterprise (EAP) | ✅ Yes |
| 5 | `WPA2_ENTERPRISE` | Alias for WPA2 Enterprise | ✅ Yes |
| 6 | `WPA3_PSK` | WPA3 with Pre-Shared Key | ✅ Yes |
| 7 | `WPA2_WPA3_PSK` | WPA2 or WPA3 (auto-negotiated) | ✅ Yes |
| 8 | `WAPI_PSK` | WAPI PSK (Chinese standard) | ✅ Yes |
| 9 | `OWE` | Opportunistic Wireless Encryption | ❌ No |
| 10 | `WPA3_ENT_192` | WPA3 Enterprise 192-bit mode | ✅ Yes |
| 11 | `WPA3_EXT_PSK` | (Deprecated) WPA3 PSK | ✅ Yes |
| 12 | `WPA3_EXT_PSK_MIXED_MODE` | (Deprecated) WPA3 Mixed | ✅ Yes |
| 13 | `DPP` | Device Provisioning Protocol | ✅ Yes |
| 14 | `WPA3_ENTERPRISE` | WPA3 Enterprise only | ✅ Yes |
| 15 | `WPA2_WPA3_ENTERPRISE` | WPA3 Enterprise Transition | ✅ Yes |

> ⚠️ Notes:
> - `WPA3_EXT_PSK` and `WPA3_EXT_PSK_MIXED_MODE` are deprecated and should be avoided in new implementations.
> - All modes except `OPEN` and `OWE` require passwords or credentials.

---

## ✅ Frontend Guidelines

- If `cipher_type == 0 (NONE)` or `authmode == 0 (OPEN)` or `9 (OWE)`, don’t prompt for a password.
- For all other values, display a password input field.
- Use lock icons to visually indicate secure networks.

---

## 📘 Example Network Scan Entry

```json
{
  "ssid": "MyNetwork",
  "rssi": -62,
  "authmode": 3,
  "cipher": 4
}
