# Chatterbox Test Client
# Sends requests to a Chatterbox server

require "socket"

chatter_box_client = TCPSocket.new("localhost", 20000)
chatter_box_client.write "create_conversation"
puts "From Chatterbox Server: " + chatter_box_client.recv(1024)
chatter_box_client.close
