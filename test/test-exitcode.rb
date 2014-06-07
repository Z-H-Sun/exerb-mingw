
#==============================================================================#
# $Id: test-exitcode.rb,v 1.3 2005/04/15 09:42:52 yuya Exp $
#==============================================================================#

require 'testcase'

#==============================================================================#

class ExitCodeTestCase < Minitest::Test
  include ExerbTestCase

  def name2
    return 'test-exitcode'
  end

  def test_exitcode
    if /mswin|mingw/ =~ RUBY_PLATFORM
      assert_equal("10\n", execute_cmd("#{@name}\\#{@name}.exe || if errorlevel 10 echo 10"))
    else
      assert_equal("10\n", execute_cmd("#{@name}/#{@name}.exe || echo $?"))
    end
  end

end # ExitCodeTestCase

#==============================================================================#
#==============================================================================#
