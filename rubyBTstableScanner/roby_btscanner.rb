require 'rubyserial'
require 'prawn'
require 'optparse'
require 'timeout'

options = {}
OptionParser.new do |opts|
  opts.banner = "Usage: scan_bt.rb [options]"

  opts.on("-d", "--duration DURATION", "Duration of the scan (e.g., '1m', '3h')") do |duration|
    options[:duration] = duration
  end

  opts.on("-o", "--output FILE", "Output PDF file") do |file|
    options[:output] = file
  end
end.parse!

raise OptionParser::MissingArgument, "Duration is required" if options[:duration].nil?
raise OptionParser::MissingArgument, "Output file is required" if options[:output].nil?

duration = options[:duration].to_i
duration = case options[:duration][-1]
           when 's' then duration
           when 'm' then duration * 60
           when 'h' then duration * 3600
           else raise "Invalid duration format"
           end

devices = []

begin
  Timeout.timeout(duration) do
    while true
      output = `hcitool scan --flush`
      output.lines.each do |line|
        if line =~ /^\s*([0-9A-Fa-f]{2}(:[0-9A-Fa-f]{2}){5})\s+(.*)$/
          address = $1
          name = $3.strip
          devices << { address: address, name: name, timestamp: Time.now } unless devices.any? { |d| d[:address] == address }
        end
      end
      sleep 1
    end
  end
rescue Timeout::Error
  puts "Scanning completed"
end

Prawn::Document.generate(options[:output]) do
  text "Bluetooth Devices Report", size: 24, style: :bold, align: :center
  move_down 20

  devices.each do |device|
    text "Device Name: #{device[:name]}"
    text "MAC Address: #{device[:address]}"
    text "Timestamp: #{device[:timestamp]}"
    move_down 10
  end
end
