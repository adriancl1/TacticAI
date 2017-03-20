# Tactic AI

## What's Tactic AI?
Tactic AI stands for Tactical Artificial Intelligence, but what does that mean? 
In RTS games like Age of Empires II, the way the enemy units act and react to our own troop is handled by tactic AI, but also how our own units do such things. 
On the other hand we have strategic AI, which would kind of represent the "enemy player", as it has control over enemy resources and other more player-oriented and controlled elements. 
The tactic AI in games defines -for example- which path will a enemy follow to get to you, taking into account your weapon, his weapon, and many other variables. You can probably see that almost every game with some kind of enemy has tactic AI .
With that in mind, there's endless ways to program our AI, and nowadays it's mostly done through scripting, pre-generated paths... However, in here we will do it the old-fashioned way.


## How to get started
First and foremost, you'll obviously need two units: one of your army and an enemy. 
They can be very basic (just a melee unit), or very complex (a flying mage) and that will decide the complexity of your AI when you program it, the possibilities are endless! 
For this research I'd recommend that they have at least a radius (how far can they spot enemies), HP and attack values. 

## Identifying enemies - Brute Force Way
So now that you've got your units, the first step would be to check if there are any enemies around them. 
The brute force way to do is to check manually all the tiles inside their radius, starting from their position and going out. For this reason, we'll use the BFS algorithm with some tweaks as it serves the purpose just right.        
For those of you who don't know or have forgotten what BFS is and how it works just read this link:      
[BFS Algorithm article](http://www.redblobgames.com/pathfinding/a-star/introduction.html)        
Depending on the radius of your unit you should get something like this:
![BFS](https://github.com/adriancl1/TacticAI/blob/master/Pictures/BFS.jpg?raw=true)

Radius of 1, 2 and 3 from left to right.        
You should check in each tile if there's a unit there, and stop calculating as you've already found the nearest enemy. 
However, this way to do it is very expensive resource-wise, and you'll have problems when many unis are deployed on the field. For now, just limit the time the calculations are done, for example, to 1 each second.

Your units should look something like this in debug mode:

![BFSUnit](https://github.com/adriancl1/TacticAI/blob/master/Pictures/UnitsBFS.gif?raw=true)  

## Identifying enemies - The Right Way
If you want to get better results, you should introduce a Quadtree system to your code.
Given the radius of your unit, you could find how many nodes are inside it and just look for the nearest enemy in said nodes (if there's any enemy). This will speed things tremendously, as you'll just check for enemies in nodes instead of check every tile in a radius. It would look a bit like this:         
![Quadtree](https://github.com/adriancl1/TacticAI/blob/master/Pictures/QuadtreeSearch.PNG?raw=true)       

If you want to implement a Quadtree, follow this research by a fellow classmate (Xavier Olivenza):       
[Quadtree Research](https://xavierolivenza.github.io/Quadtree_Point_Search_Implementation/)


## We have identified an enemy... Now what?
Now things get complicated... and kind of messy. As said before, there's endless ways to program AI, but we will follow a basich scheme for now.
Let's use a diagram to illustrate a basic fight between melee/melee, melee/ranged, ranged/ranged. 
![Diagram](https://github.com/adriancl1/TacticAI/blob/master/Pictures/AIDiagram.png?raw=true)

As we can see, there are multiple checks along the way we need to make before we begin attacking. Let's do some pseudocode to help you get the idea:

Things we should have:
* Pointer to the enemy unit
* STATE for the unit class

If the enemy's not attacking anybody else    
{    
If we're both ranged        
{        
Set both STATEs to ATTACKING       
}        
Else if I'm ranged and he's melee       
{    
Set my STATE to ATTACKING, create a path for the enemy with one of my adjacent tiles as destination       
}      
Else if I'm melee and he's ranged       
{       
Same as before, just switch the units      
}       
Else if we're both melee       
{        
Find a tile in between our positions, and a free tile next to it. This unit will go to the first one, and the enemy to the other one     
}         
}      
Else if the enemy is attacking someone else       
{          
 if his STATE==ATTACKING       
 {       
 move next to him and attack       
 }        
 else if his state is MOVING_TO_ATTACK       
 {        
 Get his destination and move to a tile next to it        
 }       
 }       
It should be noted that we use MOVING_TO_ATTACK so when our Move() function ends (we get to our destination), the unit changes its STATE to ATTACKING instead of just IDLE or NONE.

Here's how it should look like for an archer against a swordman and two swordmans against each other.

![MeleevsMelee](https://github.com/adriancl1/TacticAI/blob/master/Pictures/MeleevsMelee.gif?raw=true)
![RangedvsMelee](https://github.com/adriancl1/TacticAI/blob/master/Pictures/RangevsMelee.gif?raw=true)

## Attacking
This part's easier. You just gotta make sure that your enemy is still actually alive (his HP is above 0 and he's not a nullptr), that he's still within your range, and then use a timer to control how much damage you do per second (dps). You could also apply damage at any other rate if you wish, just use a timer (although less interval of time between damage calculations means it'll be more resource intensive if there are several units fighting all the time)!

## You're done!
When one of the units dies, the other one goes back to an IDLE state and starts searching again in his surroundings for other possible enemies. However, if for example it was a guard who defends an entrance, you could save his original position and just make him come back after defeating an enemy by generating a path to it. You may do as you wish with your AI.

## Interesting Links
[Game AI: The State of the Industry](http://www.gamasutra.com/view/feature/3570/game_ai_the_state_of_the_industry.php)       
[Game AI: The State of the Industry Part II](http://www.gamasutra.com/view/feature/131974/game_ai_the_state_of_the_.php)          
[Good examples of modern AI](http://www.gamasutra.com/view/news/269634/7_examples_of_game_AI_that_every_developer_should_study.php)
