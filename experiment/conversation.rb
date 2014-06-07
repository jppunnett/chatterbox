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
end
