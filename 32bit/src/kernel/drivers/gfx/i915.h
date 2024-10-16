#define PCI_VENDOR_INTEL 0x8086
#define PCI_DEVICE_INTEL_ATOM_N455_GFX 0xA011

#define I915_REG_GMBUS0 0x5100
#define I915_REG_GMBUS1 0x5104
#define I915_REG_GMBUS2 0x5108
#define I915_REG_GMBUS3 0x510C
#define I915_REG_GMBUS4 0x5110
#define I915_REG_GMBUS5 0x5120

typedef struct {
    struct pci_device *pci_device;
    uint32_t mmio_address;
} i915Device;