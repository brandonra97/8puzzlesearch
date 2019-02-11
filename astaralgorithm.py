"""
CSC370 HW1: A* Algorithm Implementation on 8-Puzzle

Author: Brandon Ra and Alex Hazan

"""
dim = 3 #dimension value of the board
lim = 100 #number of puzzle sample wanted for each solutiond epth

import random
import Queue
import math

class Board:
    """Class that represents a single instance of the 8-Puzzle Board

    Attributes:
        values - a 1-D array of integers that specifies the locations of the
                tiles of a puzzle board. The number goes from 0 to 8 where 0
                represents the empty tile. The first three numbers are the
                first row's tiles from left to right. Subsequent three numbers
                would be the next row, and so on.
        pCost - the path cost to the current board
        hCost - the heuristic value of the current board relative to then
                goal Board
    """
    def __init__(self, values, pCost, hCost):
        """Constructs a puzzle board based on the parameters given

        Parameters:
            values - a 1-D array of integers that specifies the locations of the
                    tiles of a puzzle board. The number goes from 0 to 8 where 0
                    represents the empty tile. The first three numbers are the
                    first row's tiles from left to right. Subsequent three numbers
                    would be the next row, and so on.
            pCost - the path cost to the current board
            hCost - the heuristic value of the current board relative to then
                    goal Board

        Returns:
            None.
        """
        self.values = values
        self.eRow = 0
        self.eCol = 0
        self.ePos = self.eRow * dim + self.eCol
        self.pCost = pCost
        self.hCost = hCost
        self.tCost = self.pCost + self.hCost

        for i in range(dim * dim):
            if self.values[i] == 0:
                self.eRow = i / dim
                self.eCol = (i % dim)

    def __hash__(self):
        """A hash function for the board class

        Returns:
            A hash for the board class
        """
        return hash(tuple(self.values))

    def __eq__(self, other):
        """A comparator method for the board class

        Parameter:
            other - the board object the self is being compared to

        Returns:
            True if the compared board object is the same and False if not
        """
        if(other == None):
            if type(self) == type(other):
                return True
            else:
                return False
        for i in range(dim * dim):
            if self.values[i] != other.values[i]:
                return False
        return True

    def __str__(self):
        """Generates and returns a string representation of this board object

        Returns:
            A string containing an ASCII representation of the current
            state of this 8-Puzzle.
        """
        string = ""
        for row in range(dim):
            temp = ""
            for col in range(dim):
                temp += "|" + str(self.values[(row * dim) + col])
            string += temp + "|\n"
        return string

    def isGoal(self, goal):
        """A method for determining if the board object is in its goal state

        Parameters:
            goal - the board object that bears the goal state

        Returns:
            None.
        """
        for i in range(dim * dim):
            if self.values[i] != goal.values[i]:
                return False
        return True

def getH(current, goal, option):
    """Generates the heuristic value for a board class

    Parameters:
        current - the board object that bears current state
        goal - the board object that bears the goal state
        option - 1 if heuristic 1
                 2 if heuristic 2

    Returns:
        Heuristic value.
    """
    if(option == 1):
        count = 0
        for i in range(dim * dim):
            if current.values[i] != goal.values[i] and goal.values[i] != 0:
                count += 1
        return count

    elif(option == 2):
        count = 0
        currentDict = {}
        goalDict = {}
        for row in range(dim):
            for col in range(dim):
                currentDict[current.values[(row * dim) + col]] = [row, col]
                goalDict[goal.values[(row * dim) + col]] = [row, col]
        for i in range((dim * dim) - 1):
            index = i + 1
            count += abs(currentDict[index][0] - goalDict[index][0]) + abs(currentDict[index][1] - goalDict[index][1])
    return count

def makeMove(board, dir):
    """Generates a new board object resultant from a move from a given board
    It works by moving a tile into the empty spot. For instance, if we choose
    to move "up", a tile will slide from the below the empty tile, into the
    empty tile.

    Parameters:
        board - the board object that bears current state
        dir - the direction in which you want to move
                0- up
                1- down
                2- right
                3- left

    Returns:
        temp - Child board made from the move.
        None - if illegal move is made
    """
#0 is up, 1 is down, 2 is right, 3 is left
    tempVals= [0 for i in range(dim * dim)]
    for row in range(dim):
        for col in range(dim):
            tempVals[row* dim + col] = board.values[(row * dim) + col]
    temp = Board(tempVals, board.pCost, board.hCost)
    if dir == 0: # up
        if temp.eRow + 1 < dim:
            temp.values[(temp.eRow * dim) + temp.eCol] = temp.values[((temp.eRow + 1) * dim) + temp.eCol]
            temp.values[((temp.eRow + 1) * dim) + temp.eCol] = 0
            temp.eRow += 1
            temp.pCost += 1
        else:
            return None
    if dir == 1: # down
        if not(temp.eRow - 1 < 0):
            temp.values[(temp.eRow * dim) + temp.eCol] = temp.values[((temp.eRow - 1) * dim) + temp.eCol]
            temp.values[((temp.eRow - 1) * dim) + temp.eCol] = 0
            temp.eRow -= 1
            temp.pCost += 1
        else:
            return None
    if dir == 2: # right
        if not(temp.eCol - 1 < 0):
            temp.values[(temp.eRow * dim) + temp.eCol] = temp.values[(temp.eRow * dim) + (temp.eCol - 1)]
            temp.values[(temp.eRow * dim) + (temp.eCol - 1)] = 0
            temp.eCol -= 1
            temp.pCost += 1
        else:
            return None
    if dir == 3: # left
        if temp.eCol + 1 < dim:
            temp.values[(temp.eRow * dim) + temp.eCol] = temp.values[(temp.eRow * dim) + (temp.eCol + 1)]
            temp.values[(temp.eRow * dim) + (temp.eCol + 1)] = 0
            temp.eCol -= 1
            temp.pCost += 1
        else:
            return None
    return temp

def aStar(start, goal, option):
    """A* algorithm function

    Parameters:
        start - the board object that bears intial state
        goal - the board object that bears the goal state
        option - 1 if heuristic 1
                 2 if heuristic 2

    Returns:
        A two-sized array where the first element is the final goal board
        state and the second element is the number of nodes generated to solve
        the board.

        None - if there is no solution
    """
    frontier = Queue.PriorityQueue()
    explored = set()
    frontier.put((start.tCost, start))
    node = start
    while(not(frontier.empty())):
        node = frontier.get()[1]
        if(node.isGoal(goal)):
            return [node, (len(explored) + frontier.qsize())]
        explored.add(node)
        for i in range(4):
            tempBoard = Board(node.values, node.pCost, node.hCost)
            childBoard = makeMove(tempBoard, i)
            if childBoard is not None:
                if childBoard not in explored:
                    childBoard.hCost = getH(childBoard, goal, option)
                    childBoard.tCost = childBoard.pCost + childBoard.hCost
                    frontier.put((childBoard.tCost, childBoard))
    return None

def randomPuzzle(start, depth):
    """Generates a random 8-Puzzle goal state based on the initial board
    given and the number of random moves to be made on the intial board.

    Parameters:
        start - the board object that bears initial state
        depth - the number of random moves to be made on the board

    Returns:
        rBoard - Random 8-Puzzle board.
    """
    rBoard = Board(start.values, 0, 0)
    visited = set()
    visited.add(rBoard)
    for i in range(depth):
        rNumDir = random.randint(0,3)
        flag = False
        while(flag == False):
            temp = Board(rBoard.values, rBoard.pCost, rBoard.hCost)
            rBoard = makeMove(rBoard, rNumDir)
            if(rBoard != None):
                if rBoard not in visited:
                    flag = True
                    visited.add(rBoard)
            else:
                rBoard = temp
                rNumDir = random.randint(0,3)
    return rBoard

def experiment(option):
    """Conducts experiment of A* algorithm and collects data on the results.

    Parameters:
        option - 1 if heuristic 1
                 2 if heuristic 2

    Returns:
        data - a dictionary that contains the results where the key is the
                solution depth and the value is an array of the number of nodes
                generated.
    """
    tested = {}
    data = {}
    evens = [24,22,20,18,16,14,12,10,8,6,4,2]
    for i in evens:
        data[i] = []
    flag = False
    count = 0
    for i in evens:
        while(len(data[i]) < lim):
            values = [0,1,2,3,4,5,6,7,8]
            random.shuffle(values)
            testStart = Board(values, 0, 0)
            testGoal = randomPuzzle(testStart, i + (i/2))
            if testStart in tested:
                if tested[testStart] == testGoal:
                    continue
            tested[testStart] = testGoal
            solved = aStar(testStart, testGoal, option)
            solution = solved[0].pCost
            numNodes = solved[1]
            if (solution in data):
                data[solution].append(numNodes)
    return data

def computeAverage(data):
    """Computes Average

    Parameters:
        data - an array of values to be averaged

    Returns:
        Average.
    """
    average = {}
    for i in data.keys():
        sum = 0
        index = 0
        for j in data[i]:
            if(index == lim):
                break
            index += 1
            sum += j
        average[i] = sum / lim
    return average

def main():

if __name__== "__main__":
    main()
