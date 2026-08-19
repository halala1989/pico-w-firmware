// BTstack configuration for the Pico W keyboard injector.
// BLE-only (no Bluetooth Classic) to keep flash/RAM usage small.
#ifndef _PICO_BTSTACK_CONFIG_H
#define _PICO_BTSTACK_CONFIG_H

// BTstack features
#define HAVE_EMBEDDED_TIME_MS
#define HAVE_MBEDTLS
#define HAVE_BTSTACK_STDIN
#define HAVE_POSIX_FILE_IO
#define HAVE_BTSTACK_MEMORY

// Bluetooth LE support; Classic disabled to save code size.
#define HAVE_BLE
// #define HAVE_CLASSIC

// BTstack configuration: buffers, sizes
#define HCI_ACL_PAYLOAD_SIZE 255
#define MAX_NR_BNEP_SERVICES 0

// LE only: keep connection pool small (single Pico W <- phone link).
#define HCI_MAX_NUM_CONNECTIONS 1
#define HCI_MAX_NUM_SM_CONNECTIONS 1

// Max number of bonded devices (we don't require bonding).
#define MAX_NR_BONDED_DEVICES 0
// Max number of LE devices in GAP whitelist etc.
#define MAX_NR_LE_DEVICES 1
// Max number of GATT clients (we are a peripheral -> 0 clients)
#define MAX_NR_GATT_CLIENTS 0
// Max number of services / characteristics / descriptors (custom service only)
#define MAX_NR_GATT_SERVICES 2
#define MAX_NR_GATT_CHARACTERISTICS 4
#define MAX_NR_GATT_DESCRIPTORS 4

// L2CAP
#define MAX_NR_L2CAP_SERVICES 2
#define MAX_NR_L2CAP_CHANNELS 1
#define MAX_NR_L2CAP_REGISTERED_PSMS 1

// SM (security manager)
#define MAX_NR_SM_LOOKUP_ENTRIES 3
#define MAX_NR_SM_MASTER_KEY_ENTRIES 3

// LE event mask enable
#define HCI_LE_EVENT_MASK_ENABLE 1

// ATT server
#define MAX_NR_ATT_SERVER_HANDLES 4
#define MAX_ATT_MTU 256

// char * vs const char *
#define CHAR_CONST const

#endif // _PICO_BTSTACK_CONFIG_H
