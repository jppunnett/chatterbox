# Chatterbox Test Client
# Sends requests to a Chatterbox server

require "socket"

puts "Connecting to Chatterbox server on port 20000"
session = TCPSocket.new("localhost", 20000)

# Chatterbox replies with Welcome message once connected and waits for a user id
puts session.gets.chomp

# Allow user to enter ID
userid = STDIN.gets.chomp
session.puts userid

# Chat until ready to leave
sender = Thread.new do
  puts "Start chatting!"
  loop do
    msg_to_send = STDIN.gets.chomp
    session.puts msg_to_send
    break if msg_to_send == "bye"
  end
  puts "sender thread ending..."
end

listener = Thread.new do
  loop do
    msg_recvd = session.gets.chomp
    puts msg_recvd
    break if msg_recvd == "bye"
  end
  puts "listener thread ending..."
end

sender.join
listener.join

session.close
