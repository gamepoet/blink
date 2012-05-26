require 'bson'

class String
  # Converts a string to a bson ObjectId.
  def to_oid
    return BSON::ObjectId.from_string(self)
  end
end
