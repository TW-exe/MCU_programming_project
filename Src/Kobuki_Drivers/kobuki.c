#include <stdint.h>
#define STM32G474xx
#include "stm32g4xx.h"
#include "uart.h"
#include "kobuki.h"
#include <stdlib.h> // include this at the top

// Parameters for distance and angle calculation
// These can be placed in kobuki.h:
#define PI (3.14159)
#define WHEEL_RADIUS (35.0) // mm
#define REVOLUTIONS_PER_TICK ((132.0 / 6545.0) / 52.0)
#define MM_PER_TICK (2.0 * PI * WHEEL_RADIUS * REVOLUTIONS_PER_TICK)
#define DEG_PER_TICK (1.0 / 100.0)
#define GAIN_ROTATE (1.0 / 0.90)
#define GAIN_DRIVE (100.0 / 100.0)
#define sign(x) ((int8_t)((x) >= 0 ? 1 : -1))

// static variables and flags, keep these here in kobuki.c
static volatile int32_t setpoint_distance_tick;
static volatile int16_t setpoint_angle_deg;
static uint8_t rotation_complete = 0;
static uint8_t distance_complete = 0;

#define HEADER_0 0xAA
#define HEADER_1 0x55

void Kobuki_Tx(uint8_t *payload, uint8_t size_payload) {
    uint8_t checksum = 0, i;
    
    UART_Tx(HEADER_0); // Send header 0
    UART_Tx(HEADER_1); // Send header 1
    UART_Tx(size_payload); // Send size of payload in bytes
    
    checksum ^= size_payload; // Calculate running checksum
    
    // Send each byte of payload one-by-one
    for (i = 0; i < size_payload; i++) {
        UART_Tx(payload[i]);
        checksum ^= payload[i];
    }
    
    UART_Tx(checksum); // Send checksum
}


uint8_t Kobuki_Rx(uint8_t *feedback, uint32_t size_feedback) {
    uint8_t feedback_header = 0, size_payload = 0, checksum = 0, rx_checksum = 0;
    uint32_t i;

    // 1. Find the valid 0xAA 0x55 sequence
    while (1) {
        feedback_header = UART_Rx();
        if (feedback_header == HEADER_0) {
            feedback_header = UART_Rx();
            if (feedback_header == HEADER_1) {
                break; // Valid start found, exit finding loop
            }
        }
    }

    // 2. Read Payload Length, first byte is size of payload in bytes
    size_payload = UART_Rx();
    checksum ^= size_payload; // checksum is XORed value of entire bytestream

    // 3. Determine safe boundary to prevent array overflow
    uint32_t limit = (size_payload < size_feedback) ? size_payload : size_feedback;

    // 4. Read into array of bytestream, this is the basic sensor data
    for (i = 0; i < limit; i++) {
        feedback[i] = UART_Rx();
        checksum ^= feedback[i];
    }

    // 5. Read and discard remaining payload (just for checksum), this is the size of rest of bytestream
    for (i = limit; i < size_payload; i++) {
        checksum ^= UART_Rx();
    }

    // 6. Verify Checksum
    rx_checksum = UART_Rx(); // checksum from kobuki - final byte
    return (checksum != rx_checksum); // 0 = Success, 1 = Error
}

// Updated Kobuki_Read() remember to replace the old wrapper with this one
void Kobuki_Read(Kobuki_Typedef *kobuki) {
    uint16_t distance_LSB;
    uint16_t distance_MSB;
    int16_t angle_LSB;
    int16_t angle_MSB;
    
    // Init buffer & checksum
    uint8_t kobuki_rx_buffer[SIZE_FEEDBACK];
    uint8_t checksum_result;
    
    // Read kobuki Tx data stream
    checksum_result = Kobuki_Rx(kobuki_rx_buffer, SIZE_FEEDBACK);
    
    // If checksum 'checks out', update sensor variables from buffer
    if (checksum_result == 0) {
        distance_LSB = kobuki_rx_buffer[DIST_LSB];
        angle_LSB = kobuki_rx_buffer[ANGLE_LSB];
        distance_MSB = (uint16_t)((uint16_t)kobuki_rx_buffer[DIST_MSB] << 8);
        angle_MSB = (int16_t)((int16_t)kobuki_rx_buffer[ANGLE_MSB] << 8);
        
        kobuki->bumper = kobuki_rx_buffer[BUMPER];
        kobuki->wheeldrop = kobuki_rx_buffer[WHEEL_DROP];
        kobuki->cliff = kobuki_rx_buffer[CLIFF];
        kobuki->button = kobuki_rx_buffer[BUTTON];
        kobuki->distance = distance_LSB | distance_MSB;
        kobuki->angle = angle_LSB | angle_MSB;
    }
    // otherwise do not update
    else {}
    
    // always update complete flags
    kobuki->rotation_complete = rotation_complete;
    kobuki->distance_complete = distance_complete;
}


void Kobuki_Rotate(int16_t speed) {
    uint8_t speed_lsb = 0;
    uint8_t speed_msb = 0;

    // Construct payload
    uint8_t payload[] = {0x01, 0x04, speed_lsb, speed_msb, 0x01, 0x00};

    // Saturation to avoid going over speed limit
    if (speed > SPEED_LIMIT) {
        speed = SPEED_LIMIT;
    } else if (speed < -SPEED_LIMIT) {
        speed = (~SPEED_LIMIT) + 1;
    }

    // Split LSB & MSB for payload
    payload[2] = (uint8_t) speed;
    payload[3] = (uint8_t) (speed >> 8);

    Kobuki_Tx(payload, sizeof(payload));
}

void Kobuki_Drive(int16_t speed) {
    uint8_t speed_lsb = 0;
    uint8_t speed_msb = 0;

    // Construct payload
    uint8_t payload[] = {0x01, 0x04, speed_lsb, speed_msb, 0x00, 0x00};

    // Saturation to avoid going over speed limit
    if (speed > SPEED_LIMIT) {
        speed = SPEED_LIMIT;
    } else if (speed < -SPEED_LIMIT) {
        speed = (~SPEED_LIMIT) + 1;
    }

    // Split LSB & MSB for payload
    // There is no truncation warning due to stdint
    payload[2] = (uint8_t) speed;
    payload[3] = (uint8_t) (speed >> 8);

    Kobuki_Tx(payload, sizeof(payload));
}

void Kobuki_Rotate_Angle_Setpoint(int16_t current_angle_tick, int16_t rot_angle_deg) {
    volatile int32_t init_angle;
    rotation_complete = 0;
    
    // initial angle
    init_angle = (int32_t)((double)(current_angle_tick + (int32_t)18000) * DEG_PER_TICK); // [-18000, 18000] -> [0, 360] deg
    
    // setpoint angle
    setpoint_angle_deg = (init_angle + rot_angle_deg + 360) % 360; // [0, 360]
}

void Kobuki_Rotate_Angle(int16_t current_angle_tick) {
    volatile int16_t speed;
    volatile int16_t error;
    volatile int32_t current_angle_deg;
    
    current_angle_deg = (int32_t)((double)(current_angle_tick + (int32_t)18000) * DEG_PER_TICK);
    error = (int16_t)(setpoint_angle_deg - current_angle_deg);
    
    // Check for non-optimal rotation angle
    if (abs(error) > 180) {
        // Always rotate with the minimum angle e.g. +270 -> -90.
        // Introduces singularity - loses sign and truncates error.
        error = -sign(error) * (abs(error) % 180);
        
        // Special cases in circular buffer. Offset to correct error.
        if ((error < 0) && (current_angle_deg < setpoint_angle_deg)) {
            error = -(error + 180);
        } else if ((error > 0) && (current_angle_deg > setpoint_angle_deg)) {
            error = -(error - 180);
            // Does this make sense? Is there a simpler way?
        }
    }
    
    if (abs(error) <= 0) {
        rotation_complete = 1;
    } else {
        speed = (sign(error) * MIN_SPEED) + (int16_t)(GAIN_ROTATE * error);
        Kobuki_Rotate(speed);
    }
}

void Kobuki_Drive_Distance_Setpoint(uint16_t current_distance_tick, int16_t distance) {
    distance_complete = 0;
    setpoint_distance_tick = current_distance_tick + (int32_t)(distance / MM_PER_TICK);
}

void Kobuki_Drive_Distance(uint16_t current_distance_tick) {
    volatile int16_t speed;
    volatile int16_t error;
    volatile uint16_t setpoint_alias;
    
    // standard condition
    error = (int16_t)((double)(setpoint_distance_tick - current_distance_tick) * MM_PER_TICK);
    setpoint_alias = (uint16_t)((setpoint_distance_tick + 0x10000) % 0x10000);
    
    if (((setpoint_distance_tick > 0xFFFF) && (current_distance_tick < setpoint_alias)) ||
        ((setpoint_distance_tick < 0) && (current_distance_tick > setpoint_alias))) {
        // alternate condition
        error = (int16_t)((double)(setpoint_alias - current_distance_tick) * MM_PER_TICK);
    }
    
    if (abs(error) <= 2) {
        distance_complete = 1;
    } else {
        speed = (sign(error) * MIN_SPEED) + (int16_t)(GAIN_DRIVE * error);
        Kobuki_Drive(speed);
    }
}

// This wrapper is based on the General Purpose Output information in the Kobuki
// bytestream document
#define EXTERN_PWR 0x30

void Kobuki_Update_LEDs(Kobuki_LED_Enum led_1_colour, Kobuki_LED_Enum led_2_colour) {
    // Split LSB & MSB for payload
    uint8_t GPO_lsb = EXTERN_PWR; // keep 5V AND 3.3V on
    uint8_t GPO_msb = led_1_colour | (uint8_t)(led_2_colour << 2);
    uint8_t payload[] = {0x0C, 0x02, GPO_lsb, GPO_msb};
    
    Kobuki_Tx(payload, sizeof(payload));
}