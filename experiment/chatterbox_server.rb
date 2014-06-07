# Chatterbox Server
# Listens for incoming requests from Chatterbox clients.
# Types of requests include:
#   Start a conversation
#   End a conversation
#   Add a message to a conversation

require "socket"
require "chatter"
require "conversation"

# A TCP/IP based server for Chatterbox.
class ChatterboxServer

  def initialize(host, port)
    @server = TCPServer.new(host, port)

    # For now, there's only a single conversation going on
    @convo = Conversation.new
  end

  def start
    puts "Chatterbox server started on port 20000"

    loop do
      puts "Waiting for incoming client requests."

      Thread.start(@server.accept) do |client|
        handle_client_connect(client)
      end
    end

    server.close
    puts "Chatterbox server stopped"
  end


  private

    def handle_client_connect(client)
      chatter_id = client.gets.chomp
      # TODO: Validate chatter ID
      # TODO: Probably need to serialize access to @convo
      @convo.add_chatter(Chatter.new(chatter_id, client))

      # Wait for messages from chatter and propagate to everyone in the conversation
      loop do
        msg = client.gets.chomp
        #TODO: Need to figure out when chatter wants to leave
        @convo.propogate_msg(msg)
      end
    end

end


chatter_box_server = ChatterboxServer.new("localhost", 20000)
chatter_box_server.start
