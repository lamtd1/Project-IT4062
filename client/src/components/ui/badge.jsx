import * as React from "react"
import { cn } from "../../lib/utils"

const badgeVariants = ({ variant = "default" } = {}) => {
    const variants = {
        default: "border-transparent bg-zinc-900 text-zinc-50 hover:bg-zinc-900/80",
        secondary: "border-transparent bg-zinc-100 text-zinc-900 hover:bg-zinc-100/80",
        destructive: "border-transparent bg-red-500 text-zinc-50 hover:bg-red-500/80",
        outline: "text-zinc-950",
    }
    return variants[variant] || variants.default
}

function Badge({ className, variant = "default", ...props }) {
    // Simple implementation without cva dependency for now to avoid installing more deps if not needed
    const variantClass = badgeVariants({ variant })

    return (
        <div className={cn("inline-flex items-center rounded-full border px-2.5 py-0.5 text-xs font-semibold transition-colors focus:outline-none focus:ring-2 focus:ring-ring focus:ring-offset-2", variantClass, className)} {...props} />
    )
}

export { Badge }
