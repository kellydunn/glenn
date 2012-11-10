#include <stdio.h>
#include <stdlib.h>
#include <libusb.h>

//SIFTEO_VENDOR_ID=0x22fa

void usb_callback(struct libusb_transfer * transfer) {
  int i;
  printf("%d bytes transferred: \n", transfer->length);

}

void poll_usb_data(libusb_device * dev){
  libusb_device_handle *handle;
  int err;
  
  libusb_open(dev, &handle);
  int kernel = libusb_kernel_driver_active(handle, 0);
  if(kernel == 1) {
    libusb_detach_kernel_driver(handle, 0);
  }

  libusb_claim_interface(handle,0);

  struct libusb_transfer * transfer;
  transfer = libusb_alloc_transfer(0);

  static unsigned char data[8];
  static int bytes_transferred;

  libusb_fill_bulk_transfer(transfer,
                            handle, 
                            0x81,
                            data,
                            sizeof(data),
                            usb_callback,
                            &bytes_transferred,
                            1000);

  err = libusb_submit_transfer(transfer);

  // TODO Need to wait until submit transfer is over
  // libusb_free_transfer(transfer);
  // libusb_release_interface(handle, 0);
  
  if(kernel == 1) {
    libusb_attach_kernel_driver(handle, 0);
  }

  //libusb_close(handle);
}

void listen_and_poll_devices(libusb_context * ctx, uint8_t * vendor_ids) {
  ssize_t count;
  libusb_device **devs;

  count = libusb_get_device_list(ctx, &devs);
  printf("%d devices detected.\n", count);

  ssize_t i;
  for(i = 0; i < count; i++) {
    struct libusb_device_descriptor desc;
    libusb_device *dev = devs[i];
    libusb_get_device_descriptor(dev, &desc);
    if(desc.idVendor == vendor_ids[0]){
      printf("Sifteo Device #%02x detected.\n", desc.idProduct);
      poll_usb_data(dev);
    }
  }  

  libusb_free_device_list(devs, 1);
}

void event_loop(libusb_context * ctx) {
  int err;
  for(;;){
    err = libusb_handle_events_completed(ctx, NULL);
    printf("ERROR: %s\n", libusb_error_name(err));
  }
}

int main (){
  // Initialize libusb context
  libusb_context * ctx;
  libusb_init(&ctx);
  libusb_set_debug(ctx, 2);

  // Create a list of interesting Vendor Ids
  // TODO accept a list of ids from client
  uint8_t vendor_ids[1]; 
  vendor_ids[0] = 0x22fa;
  listen_and_poll_devices(ctx, vendor_ids);

  // Loop for usb events
  event_loop(ctx);

  // Exit
  libusb_exit(ctx);
  return 0;
}
