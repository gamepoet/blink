module Mongo
  class DB
    def method_missing(method_id, *args)
      method_name = method_id.id2name
      len = args.length
      if method_name.chomp!('=')
        if len != 1
          raise ArgumentError, "wrong number of arguments (#{len} for 1)", caller(1)
        end
        self[method_name] = args[0]
      elsif len == 0
        self[method_name]
      else
        raise NoMethodError, "undefined method '#{method_name}' for #{self}", caller(1)
      end
    end
  end
end
