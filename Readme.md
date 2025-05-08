# Zombie Survival Game

## Description
This part is all about displaying your mastery of the various topics covered throughout the semester. You will program the AI for a player agent in a zombie survival game provided to you by the teachers. 

The agent has limited information about its surroundings. It has a rough knowledge about the world in general and the entities within its line of sight. It’s up to you to put this information to good use. 

The goal of the agent is to survive for as long as possible and get a high score. Your success depends on your ability to collect and use the helpful items scattered throughout the world and the way you handle the threat of the roaming enemy zombies.

Use the tools given to you, the concepts you’ve seen during class and other topics you’ve researched on your own. Besides having a fully working agent you will also explain its architecture on your exam presentation. In this presentation you will explain to us how you’ve built your agent, what the agent is capable of and how well it performs.

## Stages & score
The game consists of multiple stages, each more difficult than the last. Every stage lasts 60 seconds and has its own number and composition of enemies and items. Stronger enemies will become more likely to spawn in later stages and useful items less likely. Starting in stage 3, “Purge Zones” will start to spawn. These will kill everything within range after a delay. They add another layer of complexity to your exploration behavior, but possibly also opportunities to find areas without zombies! 

Additionally, your agent will get a score based on a reward and penalty system. Both the score and the stage you’ve reached serve as an indicator for the quality of your AI and as a metric to compare and compete with your fellow students! There will be a high scores channel on the official Discord server to share your progress.

### Some scoring tips:
Positive score is added for the following actions:

- Each second you survive: 1
- Killing/Hitting zombies : 15 for each kill, 5 for each hit
- Picking up items: 2

Negative score is given for:
- Missing shots: -5 (a shotgun needs to miss all its shots to be counted as a miss) 
 
## Project Requirements
For the implementation of the AI agent you must use the following techniques:
- 	At least one decision making structure (FSM, Behavior Tree, ...)
- 	Movement logic (steering behaviors, combined steering behaviors, ...)

Students are permitted to use the framework implementations of the above techniques. 

Any custom implementations or extra’s will be rewarded. A custom technique could count as a research topic for the other part of the exam.

## Evaluation criteria
-	Agent
    - World exploration
    - Movement *(steering behaviors, combined steering)*
    - Item handling
    - Enemy handling
    - Decision making
    - Extra(s)
- Code quality & structure
- Knowledge / insight: 
    - Answering questions about the written code and the topics covered in class
- GitHub Usage
    - Regular commits
    - Commit messages
