# conversation.rb

# Represents a chatterbox conversation
class Conversation
  def initialize
    @chatters = Hash.new
  end

  def add_chatter(chatter)
    # TODO: Ad error handling for detecting duplicate insertion.
    @chatters[chatter.id] = chatter
  end

  # Send the message to every chatter in the convo
  def add_msg(msg, sender)

    @chatters.each do |chatter_id, chatter|
      if chatter_id != sender.id
        chatter.client_socket.puts "#{sender.id} said: #{msg}"
      end
    end

  end

end
