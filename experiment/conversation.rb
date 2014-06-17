# conversation.rb

# Represents a chatterbox conversation
class Conversation
  def initialize
    @chatters = Hash.new
  end

  def add_chatter(chatter_to_add)
    # TODO: Ad error handling for detecting duplicate insertion.
    @chatters[chatter_to_add.id] = chatter_to_add
  end

  def remove_chatter(chatter_to_remove)
    puts "Removing #{chatter_to_remove.id} from conversation"
    @chatters.delete_if { |chatter_id, chatter| chatter_id == chatter_to_remove.id }
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
