# Chatterbox Server
# Listens for incoming requests from Chatterbox clients.
# Types of requests include:
#   Start a conversation
#   End a conversation
#   Add a message to a conversation

require "socket"
require_relative "chatter"
require_relative "conversation"

# A TCP/IP based server for Chatterbox.
class ChatterboxServer

  def initialize(host, port)
    @host = host
    @port = port

    @server = TCPServer.new(host, port)

    # For now, there's only a single conversation going on
    @convo = Conversation.new
  end

  def start
    puts "Chatterbox server started on port #{@port}."

    begin
      loop do
        puts "Waiting for incoming client requests."

        Thread.start(@server.accept) do |client|
          handle_client_connect(client)
        end
      end
    rescue Interrupt => e
      puts "Program interrupted. Exiting gracefully."
    ensure
      @server.close
    end

    puts "Chatterbox server stopped"
  end


  private

    def handle_client_connect(client)

      client.puts "Hello! Welcome to Chatterbox. Please enter a user ID."
      chatter_id = client.gets.chomp
      puts "#{chatter_id} connected."

      # TODO: Validate chatter ID
      chatter = Chatter.new(chatter_id, client)

      # TODO: Probably need to serialize access to @convo
      @convo.add_chatter(chatter)

      # Wait for messages from chatter and propagate to everyone in the conversation
      loop do
        #TODO: Need to figure out when chatter wants to leave
        msg = client.gets.chomp
        @convo.add_msg(msg, chatter)
        if msg == "bye"
          client.puts "bye"
          break
        end
      end
      
      @convo.remove_chatter(chatter)
      client.close

    end

end

Thread.abort_on_exception = true

chatter_box_server = ChatterboxServer.new("0.0.0.0", 20000)
chatter_box_server.start
