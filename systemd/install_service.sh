#!/bin/bash

# path to executable file
sudo cp ../build/kart-server /usr/local/bin/kart-server

# path to systemd unit: 
sudo cp ./kart-server.service /etc/systemd/system/kart-server.service

systemctl enable kart-server
systemctl start kart-server
systemctl daemon-reload

systemctl -l status kart-server

# systemctl stop server
# systemctl disable server