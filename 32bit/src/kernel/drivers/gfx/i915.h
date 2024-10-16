#define PCI_VENDOR_INTEL 0x8086
#define PCI_DEVICE_INTEL_ATOM_N455_GFX 0xA011

#define I915_REG_GMBUS0 0x5100
#define I915_REG_GMBUS1 0x5104
#define I915_REG_GMBUS2 0x5108
#define I915_REG_GMBUS3 0x510C
#define I915_REG_GMBUS4 0x5110
#define I915_REG_GMBUS5 0x5120

// GMBUS0
#define GMBUS_RATE_100KHZ 0
#define PIN_PAIR_LVDS 0x03
#define GMBUS0_SW_RDY (1<<30)
#define GMBUS0_READ (1<<0)

// GMBUS1
#define GMBUS_BUS_CYCLE_WAIT (1<<25)
#define EDID_ADDRESS (0x50<<1)

// GMBUS2
#define GMBUS2_HW_RDY (1<<11)
#define GMBUS2_NAK (1<<10)

typedef struct {
    struct pci_device *pci_device;
    uint32_t mmio_address;
} i915Device;