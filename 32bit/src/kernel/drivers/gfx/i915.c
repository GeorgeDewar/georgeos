#include "system.h"
#include "i915.h"

static char TAG[] = "i915";

static void i915_init_device(struct pci_device *device);
static uint32_t read_gmbus0(i915Device *device);
static uint32_t read_gmbus1(i915Device *device);
static uint32_t read_gmbus2(i915Device *device);
static uint32_t read_gmbus3(i915Device *device);
static uint32_t read_gmbus4(i915Device *device);

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
}

static void i915_read_edid(struct pci_device *pci_device) {
    // Select connector and set rate
    
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
