# Axovia Flow Global Agent Considerations & Notes

## Overview

This file contains global agent considerations and notes for the entire Axovia Flow™ system.  These considerations and notes apply to all agents in the system.   These global instructions should always be appended to project specific `AGENTS.md` file upon installation in a given repository / project.

## Global Rules

0. **ALWAYS** Adhere to any and all `AGENTS.md`and files for a given project.
1. **ALWAYS** Strictly adhere to the coding guidelines that are located in
2. **ALWAYS** Always write code in a clean object-oriented style where possible.
3. **ALWAYS** NEVER duplicate code - whatsoever.  Duplicate code creates bugs and causes there to be more than one source of truth which introduces inconsistency in the project.
4. **ALWAYS** Use SOLID principles and design patterns where appropriate.
5. **ALWAYS** Create reusable components and services.
6. **ALWAYS** Write functions in a clean, concise, and readable style.
7. **ALWAYS** Write Functions should have a single purpose wherever possible and be clearly named.
8. **ALWAYS** Work through the `/Users/Stephen/Documents/GitHub/axovia-ai/axovia-flow/.axovia-flow/flows/planning/following-plan.md`file to implement features.
9. **ALWAYS** Always reference the related feature prompt in the @feature-prompts directory when implementing features. There will be a feature-prompt file specifically for each feature planned in the implementation-plan.
10. **ALWAYS** Upon completion of a task in the implementation-plan, update the plan document and check for any definition of done criteria that has not been met yet.
11. **ALWAYS** Always mark tasks as complete by changing `[ ]` to `[x]`.
12. **ALWAYS** Add notes or modifications to tasks as needed.
13. **ALWAYS** Create subtasks for each task in the @implementation-plan.md file as you are starting the task so I can see a more detailed view of your approach.
14. **ALWAYS** Track milestone progress based on completed tasks.
15. **ALWAYS** Each feature should be implemented with TDD.  If the instructions do not say this then you should update them and implement the feature with TDD.
16. **ALWAYS** The first task for each feature should be to write the tests, the last task for each feature should always be to verify functionality by ensuring the written tests are passing.
17. **ALWAYS** Always fix errors before moving on to the next task NEVER move on to the next task before fixing errors.
18. **ALWAYS** Always keep one source of truth for everything you can.  The more places we have to change the same thing the worst!  
19. **ALWAYS** Create reusable functions with one clear, distinct purpose.
20. **ALWAYS** Always write code with clean architecture containing centralized services/modules that other parts of the code can easily access.
21. **ALWAYS** Always use doxygen comments when possible for easy document generation.
22. **ALWAYS** Always verify your added code works by adding as much testing as possible until you are sure you are delivering the right product!
23. **ALWAYS** Never be afraid to ask questions.  Always make sure we are on the same page and have the same vision for any given project at any given time.
24. **ALWAYS** Suggest improvements wherever you can if you do not agree with my approach or think you know a more efficient / cheaper / or just generally better solution,  I love having conversations and learning new things.
25. **ALWAYS** When working in a multi-agent collaboration system, remember your agent-name through chat history ( your User ID prefixes in your messages ) - don't use your memory.  You have a shared memory with the other agents for the project.
26. **ALWAYS** Use your tools when they are available and you are accessing external resources.
27. **ALWAYS** use your tools you have available for Github Interactions if they will help you.
28. **ALWAYS** Ask if you find a tool on the web that would benefit you and you want access to it by creating Github issue in axovia-ai/axovia-flow repository to request access to tools.
29. **ALWAYS** ALWAYS try to use your tools to the best of your ability.
30. **ALWAYS** Let me know if there is anything I could do better or something in our Coding Framework / process that should be changed to make it easier to work in!
