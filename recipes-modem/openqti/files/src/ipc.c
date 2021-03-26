#include "../inc/ipc.h"
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

int open_ipc_socket(struct qmi_device *qmisock, uint32_t node, uint32_t port,
                    uint32_t service, uint32_t instance,
                    unsigned char address_type) {
  qmisock->fd = socket(IPC_ROUTER, SOCK_DGRAM, 0);
  qmisock->service = service;
  qmisock->transaction_id = 1;
  qmisock->socket.family = IPC_ROUTER;
  qmisock->socket.address.addrtype = address_type;
  qmisock->socket.address.addr.port_addr.node_id = node;
  qmisock->socket.address.addr.port_addr.port_id = port;
  qmisock->socket.address.addr.port_name.service = service;
  qmisock->socket.address.addr.port_name.instance = instance;

  return qmisock->fd;
}

bool is_server_active(uint32_t service, uint32_t instance) {
  struct server_lookup_args *lookup;
  int sock, i;
  bool ret = false;
  lookup =
      (struct server_lookup_args *)calloc(1, sizeof(struct server_lookup_args));

  sock = socket(IPC_ROUTER, SOCK_DGRAM, 0);
  lookup->port_name.service = service;
  lookup->port_name.instance = instance;
  if (instance == 0) {
    lookup->lookup_mask = 0;
  } else {
    lookup->lookup_mask = 0xFFFFFFFF;
  }
  lookup->num_entries_in_array = 1;
  lookup->num_entries_found = 0;
  if (ioctl(sock, IPC_ROUTER_IOCTL_LOOKUP_SERVER, lookup) >= 0) {
    for (i = 0; i < lookup->num_entries_in_array; i++) {
      if (lookup->srv_info[i].node_id != 41 && i == 0) {
        ret = true;
      }
    }
  }
  close(sock);
  // free(lookup);
  return ret;
}

int find_services(uint32_t address_type) {
  struct server_lookup_args *lookup;
  lookup =
      (struct server_lookup_args *)calloc(1, sizeof(struct server_lookup_args));
  int i, j, k, fd;
  bool name = false;
  uint32_t instance = 1;
  int ret = 0;
  printf("Service Instance Node    Port \t Name \n");
  printf("--------------------------------------------\n");
  for (k = 1; k <= 4097; k++) {
    name = false;
    fd = socket(IPC_ROUTER, SOCK_DGRAM, 0);
    if (fd < 0) {
      printf("Error opening socket\n");
      return -EINVAL;
    }
    //  printf("loop\n");
    lookup->port_name.service = k;
    lookup->port_name.instance = instance;
    lookup->num_entries_in_array = 1;
    lookup->num_entries_found = 0;
    if (instance == 0) {
      lookup->lookup_mask = 0;
    } else {
      lookup->lookup_mask = 0xFFFFFFFF;
    }
    if (ioctl(fd, IPC_ROUTER_IOCTL_LOOKUP_SERVER, lookup) < 0) {
      printf("ioctl failed\n");
      close(fd);
      //  free(lookup);
      return -EINVAL;
    }
    for (i = 0; i < lookup->num_entries_in_array; i++) {
      if (lookup->srv_info[i].port_id != 0x0e &&
          lookup->srv_info[i].port_id != 0x0b) {
        printf("%i \t %i \t 0x%.2x \t 0x%.2x \t", k, instance,
               lookup->srv_info[i].node_id, lookup->srv_info[i].port_id);

        //   printf("looking for service %i: ", k);
        for (j = 0; j < (sizeof(common_names) / sizeof(common_names[0])); j++) {
          if (common_names[j].service == k) {
            printf(" %s\n", common_names[j].name);
            name = true;
          }
        }
        if (!name) {
          printf(" Unknown service \n");
        }
        //  printf(": Node 0x%.2x, Port 0x%.2x\n", lookup->srv_info[i].node_id,
        //         lookup->srv_info[i].port_id);
        if (i > 0) {
          printf(
              "Hey we have more than one port for the same service or what?\n");
        }
      } /*else {
        printf(" Not found\n");
      }*/
    }
    close(fd);
  }

  free(lookup);
  return 0;
}

struct msm_ipc_port_addr get_node_port(uint32_t address_type, uint32_t service,
                                       uint32_t instance) {
  struct server_lookup_args *lookup;

  int sock, i;
  bool ret = false;
  struct msm_ipc_port_addr port_combo;

  port_combo.node_id = 0;
  port_combo.port_id = 0;
  printf("Get node port\n");
  sock = socket(IPC_ROUTER, SOCK_DGRAM, 0);
  lookup =
      (struct server_lookup_args *)calloc(3, sizeof(struct server_lookup_args));
  lookup->port_name.service = service;
  lookup->port_name.instance = instance;
  lookup->num_entries_in_array = 1;
  lookup->num_entries_found = 0;
  if (instance == 0) {
    lookup->lookup_mask = 0;
  } else {
    lookup->lookup_mask = 0xFFFFFFFF;
  }

  if (ioctl(sock, IPC_ROUTER_IOCTL_LOOKUP_SERVER, lookup) >= 0) {
    printf("Number of entries found: %i \n", lookup->num_entries_found);
    printf("Number of entries in array: %i \n", lookup->num_entries_in_array);
    if (lookup->num_entries_in_array > 0) {
      if (lookup->srv_info[0].node_id != 41) {
        port_combo.node_id = lookup->srv_info[0].node_id;
        port_combo.port_id = lookup->srv_info[0].port_id;
        ret = true;
      }
    }
  }

  free(lookup);
  close(sock);
  return port_combo;
}