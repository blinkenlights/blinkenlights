#!/usr/bin/env ruby

require 'socket'
require 'ipaddr'
require 'optparse' 
require 'rdoc/ri/ri_paths'
require 'rdoc/usage'
require 'ostruct'
require 'date'


class App
  VERSION = '0.0.3'
  
  attr_reader :options

  def initialize(arguments, stdin)
    @arguments = arguments
    @stdin = stdin
    
    # Set defaults
    @options = OpenStruct.new
    @options.port = 2323
    @options.showFrames = true
  end

  # Parse options, check arguments, then process the command
  def run
        
    if parsed_options? && arguments_valid? 
      
      puts "Start at #{DateTime.now}\\n\\n" if @options.verbose

      process_arguments            
      process_command
      
      puts "\\nFinished at #{DateTime.now}" if @options.verbose
      
    else
      output_usage
    end
      
  end
  
  protected
  
    def parsed_options?
      
      # Specify options
      opts = OptionParser.new 
      opts.on('-v', '--version')    { output_version ; exit 0 }
      opts.on('-h', '--help')       { output_help ; exit 0 }
      opts.on('-n', '--noframes')   { @options.showFrames = false }
      opts.on('-t', '--timestamp')  { @options.timestamp = true }
      opts.on('-a', '--asciidisplay')  { @options.ascii = true }
      opts.on('-p', '--port PORT') do |port|
        @options.port = port
      end
      # TO DO - add additional options
            
      opts.parse!(@arguments) rescue return false
      
      process_options
      true      
    end

    # Performs post-parse processing on options
    def process_options
      @options.verbose = false if @options.quiet
    end
    
    def output_options
      puts "Options:\\n"
      
      @options.marshal_dump.each do |name, val|        
        puts "  #{name} = #{val}"
      end
    end

    # True if required arguments were provided
    def arguments_valid?
      if @arguments.length == 1
        @options.port = @arguments[0].to_i
        return true
      end
      @arguments.length == 0
    end
    
    # Setup the arguments
    def process_arguments
      # TO DO - place in local vars, etc
    end
    
    def output_help
      output_version
      RDoc::usage() #exits app
    end
    
    def output_usage
      RDoc::usage('usage') # gets usage from comments above
    end
    
    def output_version
      puts "#{File.basename(__FILE__)} version #{VERSION}"
    end
    
    def send_heartbeat
      data = [0x42424242,0,0,0].pack("NnnN")
      @socket.send(data,0)
      print Time.now.strftime("%Y-%m-%d %H:%M:%S") + ": -------> Sent ping to #{@options.proxyAddress} #{@options.proxyPort} \n"
      
      sleep(1)
    end
    
    def process_command
      
      state = Array.new(16)
      @socket = UDPSocket.new
      @socket.setsockopt(Socket::SOL_SOCKET, Socket::SO_REUSEADDR, true)
#      @socket.setsockopt(Socket::SOL_SOCKET, Socket::SO_REUSEPORT, true)

      if @options.useProxy
        print "connecting to #{@options.proxyAddress} on port #{@options.proxyPort}...\n" 
        @socket.connect(@options.proxyAddress,@options.proxyPort)
        Thread.start() do
          while true
            self.send_heartbeat
          end
        end
      else
        @socket.bind("0.0.0.0",@options.port)
        print "Listening for blinkenpackets on #{@options.port}:\n"
      end
      
      wmcus = ["192.168.103.4",
               "192.168.103.5",
               "192.168.103.6",
               "192.168.103.7",
               "192.168.103.8",
               "192.168.103.9",
               "192.168.103.10",
               "192.168.103.11",
               "192.168.103.12",
               
               "192.168.104.14",
               "192.168.104.15",
               "192.168.104.16",
               "192.168.104.17",
               "192.168.104.18",
               "192.168.104.19",
               "192.168.104.20",
               "192.168.104.21",
               "192.168.104.22",
               "192.168.104.23",
               "192.168.104.24",
               "192.168.104.25",
               
               "192.168.101.4",
               "192.168.101.5",
               "192.168.101.6",
               "192.168.101.7",
               "192.168.101.8",
               "192.168.101.9",
               "192.168.101.10",

               "192.168.102.11",
               "192.168.102.12",
               "192.168.102.13",
               "192.168.102.14",
               "192.168.102.15",
               "192.168.102.16",
               "192.168.102.17",
               "192.168.102.18",
               "192.168.102.19"
               ]
      
      loop do

        data, sender = @socket.recvfrom(10000)
        host = sender[3]
        port = sender[1]
        magic,command,lampip,is_upper = data.unpack("NNNN")
        values = data[(4*4)..data.length].unpack("NNNNNNNN")
        offset = is_upper * 8
        0.upto(7) {|d| state[d+offset] = values[d]}
        if is_upper == 1
          state.each {|d| print "#{d} "} 
          print "\n"
        end
        wmcus.each do |address|
          @socket.send(data, 0,host,port)
          UDPSocket.open.send(data, 0, address, 2323)          
        end
      end
      
    rescue SignalException
      # only for easy exit
      print "\n"
    end

end


# Create and run the application
app = App.new(ARGV, STDIN)
app.run

