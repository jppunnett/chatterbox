# chatter.rb

# Represents a Chatterbox client
class Chatter
  attr_reader :id, :client_socket
  def initialize(id, client_socket)
    @id = id
    @client_socket = client_socket    
  end
end

