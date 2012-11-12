#include <stdio.h>
#include <stdlib.h>
#include <libusb.h>
#include <lo/lo.h>

//SIFTEO_VENDOR_ID=0x22fa
void usb_callback(struct libusb_transfer * transfer) {
  int i;
  printf("%d bytes transferred: \n", transfer->length);
  printf("data: %s\n", transfer->buffer);
  libusb_free_transfer(transfer);
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

  //libusb_release_interface(handle, 0);
  
  if(kernel == 1) {
    libusb_attach_kernel_driver(handle, 0);
  }

  //libusb_close(handle);
}

void listen_and_poll_devices(libusb_context * ctx, uint16_t * vendor_ids) {
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
}

void event_loop(libusb_context * ctx, const struct libusb_pollfd ** list) {
  int err;
  for(;;){
    poll(&list, 0, 0);
    err = libusb_handle_events_completed(ctx, NULL);
    printf("ERROR: %s\n", libusb_error_name(err));
  }
}

int main (){
  // Initialize libusb context
  libusb_context * ctx;
  libusb_init(&ctx);
  libusb_set_debug(ctx, 2);

  const struct libusb_pollfd ** poll_fds = libusb_get_pollfds(ctx);
  
  // Initialize OSC Connections
  // TODO Be able to configure multiple OSC connections
  lo_address server = lo_address_new("127.0.0.1", "8080");

  // Create a list of interesting Vendor Ids
  // TODO accept a list of ids from client
  uint16_t vendor_ids[1]; 
  vendor_ids[0] = 0x22fa;
  listen_and_poll_devices(ctx, vendor_ids);

  // Loop for usb events
  event_loop(ctx, poll_fds);

  // Exit
  libusb_exit(ctx);
  return 0;
}
