#include "system.h"
#include "i915.h"

static char TAG[] = "i915";

static void i915_init_device(struct pci_device *device);
static bool i915_read_edid(i915Device *device);
static bool i915_ddc_request_edid(i915Device *device);
static bool i915_ddc_read(i915Device *device, void *buffer, int length);
static bool wait_for_gmbus_hwready(i915Device *device);
static uint32_t read_gmbus0(i915Device *device);
static uint32_t read_gmbus1(i915Device *device);
static uint32_t read_gmbus2(i915Device *device);
static uint32_t read_gmbus3(i915Device *device);
static uint32_t read_gmbus4(i915Device *device);
static void write_gmbus0(i915Device *device, uint32_t value);
static void write_gmbus1(i915Device *device, uint32_t value);
static void write_gmbus2(i915Device *device, uint32_t value);
static void write_gmbus3(i915Device *device, uint32_t value);
static void write_gmbus4(i915Device *device, uint32_t value);

void i915_init() {
    // Check for i915 graphics card
    struct pci_device *device;
    bool found = false;
    for(int i=0; i<pci_device_count; i++) {
        device = &pci_devices[i];
        if (device->vendor_id == PCI_VENDOR_INTEL && device->device_id == PCI_DEVICE_INTEL_ATOM_N455_GFX) {
            found = true;
            i915_init_device(device);
        }
    }

    if (!found) return FAILURE;

    log_level = INFO;
    return SUCCESS;
}

static void i915_init_device(struct pci_device *pci_device) {
    log_level = DEBUG; // for now
    kprintf(INFO, TAG, "Detected i915-compatible graphics device\n");

    uint32_t bar0 = pci_get_bar32(pci_device, 0);
    uint32_t bar1 = pci_get_bar32(pci_device, 1);
    uint32_t bar2 = pci_get_bar32(pci_device, 2);
    uint32_t bar3 = pci_get_bar32(pci_device, 3);
    kprintf(DEBUG, TAG, "BAR0: %8x, BAR1: %4x, BAR2: %8x, BAR3: %8x\n", bar0, bar1, bar2, bar3);

    i915Device *device = malloc(sizeof(i915Device));
    device->pci_device = pci_device;
    device->mmio_address = bar0;

    uint32_t gmbus0 = read_gmbus0(device);
    uint32_t gmbus1 = read_gmbus1(device);
    uint32_t gmbus2 = read_gmbus2(device);
    uint32_t gmbus3 = read_gmbus3(device);
    uint32_t gmbus4 = read_gmbus4(device);
    kprintf(DEBUG, TAG, "GMBUS0: %8x, GMBUS1: %8x, GMBUS2: %8x, GMBUS3: %8x, GMBUS4: %8x\n", 
        gmbus0, gmbus1, gmbus2, gmbus3, gmbus4);

    i915_read_edid(device);
}

static bool i915_read_edid(i915Device *device) {
    // Select connector and set rate
    write_gmbus0(device, PIN_PAIR_LVDS | GMBUS_RATE_100KHZ);

    // Send the request
    if (i915_ddc_request_edid(device) < 0) return FAILURE;

    // Read the data
    char *buffer = malloc(128);
    memset(buffer, 0, 128);
    if (i915_ddc_read(device, buffer, 128) < 0) return FAILURE;
    dump_mem8(stdout, "EDID: ", buffer, 128);
}

static bool i915_ddc_request_edid(i915Device *device) {
    kprintf(DEBUG, TAG, "Requesting EDID data\n");
    int length = 1;

    // Write the 1 bytes request (0) to the GMBUS3 register
    write_gmbus3(device, 0);

    // Write the destination address, the byte count and the software-ready bit to the GMBUS1 register
    write_gmbus1(device, EDID_ADDRESS | (length << 16) | GMBUS_BUS_CYCLE_WAIT | GMBUS0_SW_RDY);

    // Wait for HW_RDY
    if (wait_for_gmbus_hwready(device) < 0) return FAILURE;

    return SUCCESS;
}

static bool i915_ddc_read(i915Device *device, void *buffer, int length) {
    kprintf(DEBUG, TAG, "Reading %d bytes from I2C\n", length);
    if (length % 4 != 0) return FAILURE; // Keep things simple

    // Write the destination address, the byte count and the software-ready bit to the GMBUS1 register
    write_gmbus1(device, EDID_ADDRESS | (length << 16) | GMBUS_BUS_CYCLE_WAIT | GMBUS0_SW_RDY | GMBUS0_READ);
    kprintf(DEBUG, TAG, "GMBUS1: %8x\n", read_gmbus1(device));

    int num_packets = length / 4;
    for(int packet=0; packet<num_packets; packet++) {
        // Wait for HW_RDY
        if (wait_for_gmbus_hwready(device) < 0) return FAILURE;

        // Read 4 bytes of data from GMBUS3
        uint32_t data = read_gmbus3(device);
        *(uint32_t *) buffer = data;

        // Increment buffer
        buffer += 4;
    }

    return SUCCESS;
}

static bool wait_for_gmbus_hwready(i915Device *device) {
    kprintf(DEBUG, TAG, "Waiting for HW_RDY\n");
    
    const int timeout = 500, interval = 100;
    for(int i=0; i<timeout; i+=interval) {
        uint32_t gmbus2 = read_gmbus2(device);
        kprintf(DEBUG, TAG, "GMBUS2: %8x\n", gmbus2);

        if (gmbus2 & GMBUS2_HW_RDY) {
            kprintf(DEBUG, TAG, "HW_RDY (waited %dms)\n", i);
            return SUCCESS;
        }
        if (gmbus2 & GMBUS2_NAK) {
            kprintf(DEBUG, TAG, "NAK (waited %dms)\n", i);
            return FAILURE;
        }
        delay(interval);
    }

    kprintf(DEBUG, TAG, "HW_RDY timed out\n");
    return FAILURE;
}

static uint32_t read_gmbus0(i915Device *device) {
    uint32_t *val = ((char *) device->mmio_address) + I915_REG_GMBUS0;
    return *val;
}

static uint32_t read_gmbus1(i915Device *device) {
    uint32_t *val = ((char *) device->mmio_address) + I915_REG_GMBUS1;
    return *val;
}

static uint32_t read_gmbus2(i915Device *device) {
    uint32_t *val = ((char *) device->mmio_address) + I915_REG_GMBUS2;
    return *val;
}

static uint32_t read_gmbus3(i915Device *device) {
    uint32_t *val = ((char *) device->mmio_address) + I915_REG_GMBUS3;
    return *val;
}

static uint32_t read_gmbus4(i915Device *device) {
    uint32_t *val = ((char *) device->mmio_address) + I915_REG_GMBUS4;
    return *val;
}

static void write_gmbus0(i915Device *device, uint32_t value) {
    uint32_t *val = ((char *) device->mmio_address) + I915_REG_GMBUS0;
    *val = value;
}

static void write_gmbus1(i915Device *device, uint32_t value) {
    uint32_t *val = ((char *) device->mmio_address) + I915_REG_GMBUS1;
    *val = value;
}

static void write_gmbus2(i915Device *device, uint32_t value) {
    uint32_t *val = ((char *) device->mmio_address) + I915_REG_GMBUS2;
    *val = value;
}

static void write_gmbus3(i915Device *device, uint32_t value) {
    uint32_t *val = ((char *) device->mmio_address) + I915_REG_GMBUS3;
    *val = value;
}

static void write_gmbus4(i915Device *device, uint32_t value) {
    uint32_t *val = ((char *) device->mmio_address) + I915_REG_GMBUS4;
    *val = value;
}