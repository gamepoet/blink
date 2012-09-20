#!/usr/bin/env ruby

require 'bundler/setup'
Bundler.require(:default)
require 'ostruct'
require 'pathname'
require 'pp'

#
# Command-line tool to convert type definitions described in JSON and generate
# various output formats:
#
# - structs in C Headers
# - compact binary schema
#

VERSION = 1

TYPE_BIT_COUNT  = 4
SPAN_BIT_COUNT  = 10

TYPE_BIT_MASK   = (1 << TYPE_BIT_COUNT) - 1
SPAN_BIT_MASK   = (1 << SPAN_BIT_COUNT) - 1

TYPES = %w(
  int8_t
  uint8_t
  int16_t
  uint16_t
  int32_t
  uint32_t
  float
).map { |x| x.to_sym }

TYPE_SIZES = %w(
  1
  1
  2
  2
  4
  4
).map { |x| x.to_i }

class Schema
  attr_reader :name, :members
  attr_accessor :header

  def initialize(name)
    @name = name
    @members = []
  end
end

class SchemaMember
  attr_reader :name, :type

  def initialize(name, type)
    @name = name
    @type = type
  end
end

class SchemaDb
  def initialize
    @db = {}
  end

  def []=(name, schema)
    if @db.include?(name)
      raise "Duplicate schema: #{name}"
    end
    @db[name] = schema
  end

  def [](name)
    return @db[name]
  end

  def headers
    return @db.values.map { |x| x.header }.uniq.sort
  end

  def types_for_header(header)
    header = Pathname.new(header.downcase)
    matches = @db.values.select { |x| Pathname.new(x.header.downcase) == header }
    matches.sort!
    return matches
  end
end

class Templates
  def self.load!
    begin
      data = IO.binread(DATA)
    rescue Errno::ENOENT
      data = nil
    end

    @templates = {}
    if data
      template = nil
      data.each_line do |line|
        if line =~ /^@@\s*(.*\S)\s*$/
          template = ''
          @templates[$1.to_sym] = template
        elsif template
          template << line
        end
      end
    end
  end

  def self.render(name, locals={})
    return Erubis::Eruby.new(@templates[name]).result(locals)
  end
end

def parse(filenames)
  schemas = SchemaDb.new

  filenames.each do |filename|
    dirname = File.dirname(filename)

    # read in the file
    json = JSON.parse(IO.read(filename))

    # check the version
    raise 'no version info in file' unless json.include?('version')
    raise "version mismatch (expected #{VERSION}, got #{json['version']}" unless json['version'] == VERSION

    # extract options
    header = json['header']

    # extract types
    json_types = json['types'] || {}
    json_types.each do |name, json_schema|
      schema = Schema.new(name)
      schema.header = json_schema['header'] || header

      json_members = json_schema['members'] || []
      json_members.each do |json_member|
        member_name = json_member['name']
        member_type = json_member['type']
        raise "#{name} has member with no name" if member_name.nil?
        member_name.strip!
        raise "#{name}.#{member_name} missing type" if member_type.nil?
        member_type = member_type.strip.to_sym
        raise "#{name} has duplicate member '#{member_name}'" if schema.members.include?(member_name)

        schema.members << SchemaMember.new(member_name, member_type)
      end

      schemas[name] = schema
    end
  end

  return schemas
end

def write_headers(schemas)
  headers = schemas.headers
  headers.each do |header|
    types = schemas.types_for_header(header)
    data = Templates.render(:header, { :filename => header, :structs => types })
    IO.write(header, data)
  end
end

def write_schema(schemas)
  schemas.each do |schema|
    schema.members.each do |member|
      name_hash = Hashes::Murmur3.hash(schema.name, 0)
      type_id   = TYPES.index(schema.type)
      span      = 1

      # pack the data and write it out
      int0      = name_hash
      int1      = (type_id & TYPE_BIT_MASK) | ((span & SPAN_BIT_MASK) << TYPE_BIT_COUNT)
      io.write([int0, int1].pack('LL'))
    end
  end
end

def main
  Templates.load!

  files = ARGV
  schemas = parse(files)
  write_headers(schemas)
end

main if __FILE__ == $0

__END__

@@ header
// AUTO-GENERATED TYPES FILE

#pragma once

<% structs.each do |s| %>
struct <%= s.name %> {
  <% s.members.each do |m| %>
  <%= m.type %> <%= m.name %>;
  <% end %>
};

<% end %>
