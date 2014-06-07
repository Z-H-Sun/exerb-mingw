
#==============================================================================#
# $Id: test-socket.rb,v 1.1 2006/12/29 08:46:25 arton Exp $
#==============================================================================#

require 'testcase'

#==============================================================================#

class SocketTestCase < Minitest::Test
  include ExerbTestCase

  def name2
    return 'test-socket'
  end

  def test_socket
    assert_nothing_thrown("socket") do 
      read_result(@name)
    end
    assert(system('test-socket\test-socket.exe'))
  end

end # ArgvTestCase

#==============================================================================#
#==============================================================================#
