# Scheduler Choice

poco comes with 2 basic schedulers, round-robin and priority. The user is free to choose
one to use within their projects depending on the application.

These scheduler types are not unique to this library, and the tradeoffs here are also
universal with any round-robin/priority scheduling problem.

Round-Robin

- Prevents resource starvation.
- Low responsiveness.

Priority

- High responsiveness.
- Susceptible to resource starvation (if high priority coroutine never blocks)
- Introduces potential for prioirity inversion.
