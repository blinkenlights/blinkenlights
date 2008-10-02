#!/usr/bin/env ruby

# == Synopsis 
#   blinkenmultiframer listens on a port for blinkenpackets and sends packets converted to 
# == Examples
#   blinkenmultiframer.rb -p 2323 -o 2323 -b 4 proxy.blinkenlights.net
#
#   Other examples:
#     blinkenmultiframer -n -p 2324
# == Usage 
#   blinkenmultiframer.rb [-v | -h | -p PORT | -o PORT] [-n] [-s] [targetAddress]
#
#   For help use: blinkenmultiframer.rb -h
# == Options
#   -h, --help               Displays help message
#   -v, --version            Display the version, then exit
#   -p, --port PORT          Port to listen to (default 2323)
#   -o, --outport PORT       Port to send packets to (default 2324)
#   -n, --noframes           Don't ouptut frames
#   -b, --bitsperpixel BITS  4 or 8
#   -f, --alwaysfull         Don't split up big screens into small ones
#   -s, --silent             Don't output anything
# == Author
#   Dominik Wagner
# == Copyright
#   Frei und so.

require 'socket'
require 'ipaddr'
require 'optparse' 
require 'rdoc/ri/ri_paths'
require 'rdoc/usage'
require 'ostruct'
require 'date'


class App
  VERSION = '0.1'
  
  attr_reader :options

  def initialize(arguments, stdin)
    @arguments = arguments
    @stdin = stdin
    
    # Set defaults
    @options = OpenStruct.new
    @options.port = 2323
    @options.outport = 2324
    @options.bits    = 8
    @options.showFrames = true
    @options.alwaysFull = false
    @options.targetAddress = "localhost"
    @options.silent = false
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
      opts.on('-s', '--silent')     { @options.silent = true }
      opts.on('-n', '--noframes')   { @options.showFrames = false }
      opts.on('-f', '--alwaysfull') { @options.alwaysFull = true }
      opts.on('-b', '--bitsperpixel BITS') do |bits|
        if (bits.to_i == 4)
          @options.bits = 4
        end
      end
      opts.on('-p', '--port PORT') do |port|
        @options.port = port
      end
      opts.on('-o', '--outport PORT') do |port|
        @options.outport = port
      end
      opts.on('-o', '--outport PORT') do |port|
        @options.outport = port
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
      # TO DO - implement your real logic here
      if @arguments.length == 0
        true
      elsif @arguments.length == 1
        @options.targetAddress = @arguments[0]
        true
      else
        false
      end
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
    
    def dataforsubframe_with_origin_and_position(inData,inOriginX,inOriginY,inWidth,inHeight,originalWidth)
      # (0,0) is top left
      result = ""
      position = inOriginY * originalWidth
      while (inHeight > 0)
        result << inData[(position + inOriginX)...(position + inOriginX + inWidth)]
        position += originalWidth
        inHeight -= 1
      end
      
      result
    end
    
    def nibbledata_for_maxval_and_width(inData,inMaxval,inWidth)
      result = ""
      position = 0;
      while (position < inData.length)
        rowstart = position
        while (position <= rowstart + inWidth - 2)
           firstnibble  = (inData[position] * 15.0 / inMaxval).round
          secondnibble  = (inData[position+1] * 15.0 / inMaxval).round
          result << (firstnibble<<4)+secondnibble
          position += 2
        end
        if (position < rowstart + inWidth)
          result = (inData[position] * 15.0 / inMaxval).round << 4
          position += 1
        end
      end
      result
    end
    
    def process_command
      
      print "Listening for blinkenpackets on #{@options.port} - and sending them to #{@options.targetAddress}:#{@options.outport}\n"
      
      socket = UDPSocket.new
      socket.setsockopt(Socket::SOL_SOCKET, Socket::SO_REUSEADDR, true)
      socket.setsockopt(Socket::SOL_SOCKET, Socket::SO_REUSEPORT, true)
      socket.bind("0.0.0.0",@options.port)
      addr  = '0.0.0.0'
      host  = Socket.gethostname
      # socket.setsockopt(Socket::IPPROTO_IP, Socket::IP_ADD_MEMBERSHIP, mreq)
      loop do
        data, sender = socket.recvfrom(10000)
        host = sender[3]
        magic = data.unpack('N')
        now = Time.now
        timestamp = (now.to_f * 1000).to_i

        print "<#{host}>: Received packet (#{data.length} bytes), magic: 0x#{"%08x" % magic[0]} \n" unless @options.silent
        if (magic[0] == 0x23542666) 
      
      # struct mcu_frame_header
      # {
      #   int32_t magic;     /* == MAGIC_MCU_FRAME                        */
      #   int16_t height;    /* rows                                      */
      #   int16_t width;     /* columns                                   */
      #   int16_t channels;  /* Number of channels (mono/grey: 1, rgb: 3) */
      #   int16_t maxval;    /* maximum pixel value (only 8 bits used)    */
      #   /*
      #    * followed by
      #    * unsigned char data[rows][columns][channels];
      #    */
      # };
          magic,height,width,channels,maxval = data.unpack('Nnnnn');
          baseByte = 12
          print "          MAGIC_MCU_FRAME #{width}x#{height} channel#:#{channels} maxval:#{maxval} - content length:#{data.length-baseByte} bytes\n" unless @options.silent
          if (@options.showFrames && !@options.silent)
            heightToReduce = height
            formatString = (maxval > 15) ? "%02x " : "%01x "
            while (heightToReduce > 0) 
              print "          "
              data[baseByte...(baseByte + width)].each_byte { |c| print formatString % c;}
              print "\n"
              heightToReduce = heightToReduce - 1
              baseByte = baseByte + width
            end
          end
          

          # build my data
          outdata = [0x23542668,timestamp>>32,timestamp & 0xFFFFFFFF].pack('NNN')
          pixeldata = data[12...(data.length)]
          if (!@options.alwaysFull && width == 96 && height == 32)
          #this is steroscope - lets cut it up ;)
            for screen in [{:screenID => 5, :width => 22, :height =>17, :originY => 11, :originX => 16},
                           {:screenID => 6, :width => 30, :height =>23, :originY =>  5, :originX => 50}]
              subdata = self.dataforsubframe_with_origin_and_position(data[12...(data.length)],
                screen[:originX],screen[:originY],
                screen[:width],  screen[:height], width)
              outdata << [screen[:screenID],@options.bits,screen[:height],screen[:width]].pack('CCnn')
              if (@options.bits == 4)
                outdata << self.nibbledata_for_maxval_and_width(subdata,maxval,screen[:width])
              else
                subdata.each_byte {|byte| outdata << (byte * 255.0 / maxval).round }
              end
            end
          else
            # if not stereoscope just put everything into screen number 0
            outdata << [0,@options.bits,height,width].pack('CCnn')
            if (@options.bits == 4)
              outdata << self.nibbledata_for_maxval_and_width(pixeldata,maxval,width)
            else
              pixeldata.each_byte {|byte| outdata << (byte * 255.0 / maxval).round }
            end
          end

          UDPSocket.open.send(outdata, 0, @options.targetAddress, @options.outport)          
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

