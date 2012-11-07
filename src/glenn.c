#include <stdio.h>
#include <stdlib.h>
#include <libusb.h>

//SIFTEO_VENDOR_ID=0x22fa

void usb_callback(struct libusb_transfer * transfer) {
  int i;
  printf("%d bytes transferred: \n", transfer->length);
  printf("result: %s\n",transfer->buffer);
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


  char data[8];
  int bytes_transferred;
  libusb_fill_bulk_transfer(transfer,
                            handle, 
                            0x81,
                            data,
                            sizeof(data),
                            usb_callback,
                            &bytes_transferred,
                            1000);

  libusb_submit_transfer(transfer);
  //libusb_free_transfer(transfer);
  //libusb_release_interface(handle, 0);
  
  if(kernel == 1) {
    libusb_attach_kernel_driver(handle, 0);
  }

  //libusb_close(handle);
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

  int err;
  for(;;){
    err = libusb_handle_events_completed(ctx, NULL);
    printf("ERROR: %s\n", libusb_error_name(err));
  }

  libusb_free_device_list(devs, 1);
  libusb_exit(ctx);
  return 0;
}
