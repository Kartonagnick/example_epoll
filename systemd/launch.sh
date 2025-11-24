clear

systemctl -l status kart-server
systemctl start kart-server

gnome-terminal -- bash -c "../build/kart-client_tcp; exec bash"
gnome-terminal -- bash -c "../build/kart-client_tcp; exec bash"
gnome-terminal -- bash -c "../build/kart-client_udp; exec bash"
