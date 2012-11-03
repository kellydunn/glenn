#include <stdio.h>
#include <libusb.h>

//SIFTEO_VENDOR_ID=0x22fa

void poll_usb_data(libusb_device * dev){
  libusb_device_handle *handle;
  int err;
  
  libusb_open(dev, &handle);
  int kernel = libusb_kernel_driver_active(handle, 0);
  if(kernel == 1) {
    libusb_detach_kernel_driver(handle, 0);
  }
  libusb_claim_interface(handle,0);

  char data[8];
  int bytes_transferred;
  libusb_bulk_transfer(handle, 
                       0x81,
                       data,
                       sizeof(data),
                       &bytes_transferred,
                       1000);
  
  libusb_release_interface(handle, 0);
  
  if(kernel == 1) {
    libusb_attach_kernel_driver(handle, 0);
  }

  printf("%d bytes transferred: %s\n", bytes_transferred, data);
  libusb_close(handle);
}

int main (){
  libusb_context * ctx;
  libusb_init(&ctx);

  libusb_set_debug(ctx, 2);

  ssize_t count;
  libusb_device **devs;
  count = libusb_get_device_list(ctx, &devs);
  printf("%d devices detected.\n", count);

  ssize_t i;
  for(i = 0; i < count; i++) {
    struct libusb_device_descriptor desc;
    libusb_device *dev = devs[i];
    libusb_get_device_descriptor(dev, &desc);
    if(desc.idVendor == 0x22fa){
      printf("Sifteo Device #%02x detected.\n", desc.idProduct);
      poll_usb_data(dev);
    }
  }

  libusb_free_device_list(devs, 1);
  libusb_exit(ctx);
  return 0;
}
