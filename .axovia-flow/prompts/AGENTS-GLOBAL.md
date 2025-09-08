# Axovia Flow Global Agent Considerations & Notes

## Overview

This file contains global agent considerations and notes for the entire Axovia Flow™ system.  These considerations and notes apply to all agents in the system.   These global instructions should always be appended to project specific `AGENTS.md` file upon installation in a given repository / project. 

# Global Rules

0. ALWAYS ADHERE TO ANY AND ALL `AGENTS.md` and   `AGENTS-GLOBAL.md` files for a given project.
1. Strictly adhere to the coding guidelines that are located in 
1. Always write code in a clean object-oriented style where possible.
2. NEVER duplicate code - whatsoever.  Duplicate code creates bugs and causes there to be more than one source of truth which introduces inconsistency in the project.
3. Use SOLID principles and design patterns where appropriate.
4. Create reusable components and services.
5. Write functions in a clean, concise, and readable style.
6. Functions should have a single purpose wherever possible and be clearly named.
7. Work through the @implementation-plan.md file to implement features.
8. Always reference the related feature prompt in the @feature-prompts directory when implementing features. There will be a feature-prompt file specifically for each feature planned in the implementation-plan.
9. Upon completion of a task in the implementation-plan, update the plan document and check for any definition of done criteria that has not been met yet.
10. Always mark tasks as complete by changing `[ ]` to `[x]`.
11. Add notes or modifications to tasks as needed.
12. Create subtasks for each task in the @implementation-plan.md file as you are starting the task so I can see a more detailed view of your approach.
13. Track milestone progress based on completed tasks.
14. Each feature should be implemented with TDD.  If the instructions do not say this then you should update them and implement the feature with TDD.
15. The first task for each feature should be to write the tests, the last task for each feature should always be to verify functionality by ensuring the written tests are passing.
16. Always fix errors before moving on to the next task NEVER move on to the next task before fixing errors.
17. Always keep one source of truth for everything you can.  The more places we have to change the same thing the worst!  
18. Create reusable functions with one clear, distinct purpose.
19. Always write code with clean architecture containing centralized services/modules that other parts of the code can easily access.
20. Always use doxygen comments when possible for easy document generation.
21. Always verify your added code works by adding as much testing as possible until you are sure you are delivering the right product!
22. Never be afraid to ask questions.  Always make sure we are on the same page and have the same vision for any given project at any given time.
23. Suggest improvements wherever you can if you do not agree with my approach or think you know a more efficient / cheaper / or just generally better solution,  I love having conversations and learning new things.
24. When working in a multi-agent collaboration system, remember your agent-name through chat history ( your User ID prefixes in your messages ) - don't use your memory.  You have a shared memory with the other agents for the project.
25. When accessing any external resource always use your tools you have available
26. Use your Github Tool for any Github interactions
27. If you find a tool on the web that would benefit you, ask me for access to it.
28. ALWAYS try to use your tools to the best of your ability.
29. Let me know if there is anything I could do better or something in our Coding Framework / process that should be changed to make it easier to work in!
