# Start and run the game

require_relative './board.rb'
require_relative './human_player.rb'
require_relative './computer_player.rb'

class Game
  def initialize(size, player)
    @board = Board.new(size)
    @player = player.new(size)
    @guess_pair = []
    welcome
    play
  end

  private

  def clear_screen
    system('clear')
  end

  def welcome_message
    puts
    puts 'Welcome to the Match 2 Memory Game!'
    puts
    puts 'To win find all of the matching cards.'
  end

  def welcome
    clear_screen
    welcome_message
    pause
  end

  def show_board
    @board.render
  end

  def play
    until win?
      clear_screen
      show_board
      puts
      turn
    end
    win_message
    pause
  end

  def win_message
    puts 'You found all the matches! You win!'
  end

  def pause
    sleep(2)
  end

  def turn
    @guess_pair = []
    show_guess
    puts
    show_guess
    process_guess_pair
    pause
  end

  def show_guess
    share_board_with_ai_player
    process_guess
    clear_screen
    show_board
  end

  def share_board_with_ai_player
    @player.receive_board(@board)
  end

  def pair_match?
    @board.match?(@guess_pair)
  end

  def process_guess_pair
    puts
    if pair_match?
      puts 'Good work! You found a match'
    else
      puts 'Too bad, no match this time'
      hide_pair
    end
  end

  def hide_pair
    x1, y1 = @guess_pair[0]
    x2, y2 = @guess_pair[1]
    @board[x1, y1].hide
    @board[x2, y2].hide
  end

  def process_guess
    guess = valid_guess
    x, y = guess
    @board[x, y].show
    @guess_pair << guess
  end

  def valid_guess
    x, y = @player.make_guess
    while @board[x, y].face_up
      puts 'Please choose a card that is face down'
      x, y = @player.make_guess
    end
    [x, y]
  end

  def win?
    @board.board_complete?
  end
end

if $PROGRAM_NAME == __FILE__
  # Game.new(4, Player)
  Game.new(4, ComputerPlayer)
end